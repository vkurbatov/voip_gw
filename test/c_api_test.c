/* Тестовое приложение для проверки c-api библиотеки voip_gw.
 * Приложение максимально урощено, обрабатывается только один звонок
 * входящий или исходящий. Целью приложения является демонстрация,
 * поэтому для простоты используются глобальные переменные и
 * циклы опроса переменных.
 * Тест тупо заворачивает обратно аудио и видео потоки, а также
 * DTMF-сигналы.
 */

#include "c_api_test.h"
#include "c_api/voip_api.h"

#include <unistd.h>
#include <stdio.h>

#define __defult_queue_size 5
#define __default_read_queue_timeout_ms 100

uint32_t stop = 0;                          // признак завершения программы (когда будет отбит звонок)

vgw_handle_t call_handle = vgw_failed;      // дескриптор звонка
vgw_handle_t audio_queue = vgw_failed;      // дескриптор аудио очереди
vgw_handle_t video_queue = vgw_failed;      // дескриптор видео-очереди

// обработчик события запуска менеджера звонков
static vgw_result_t on_manager_started(vgw_handle_t handle)
{
    return vgw_ok;
}

// обработчик события остановки менеджера звонков
static vgw_result_t on_manager_stopped(vgw_handle_t handle)
{
    return vgw_ok;
}

// обработчик события нового звонка
static vgw_result_t on_manager_create_call(vgw_handle_t handle
                                           , struct vgw_call_info_t* call_info)
{
    // если звонка ранее не было
    if (__vgw_is_result_failed(call_handle))
    {
        // сохраним новый дескриптор
        call_handle = handle;
        return vgw_ok;
    }

    return vgw_failed;
}

// обработчик события отбоя звонка
static vgw_result_t on_manager_release_call(vgw_handle_t handle
                                            , struct vgw_call_info_t* call_info)
{
    // проверям наш ли звонок
    if (call_handle == handle)
    {
        call_handle = vgw_failed;
        stop = 1;                   // после отбоя приложению делать больше нечего
        return vgw_ok;
    }
    return vgw_failed;
}

// диспетчеризация событий менеджера звонков
inline static vgw_result_t on_manager_message(struct vgw_manager_message_t* manager_message)
{
    switch(manager_message->event_type)
    {
        case vgw_manager_event_started:
            return on_manager_started(manager_message->handle);
        break;
        case vgw_manager_event_stopped:
            return on_manager_stopped(manager_message->handle);
        break;
        case vgw_manager_event_create_call:
            return on_manager_create_call(manager_message->handle, &manager_message->body.call_info);
        break;
        case vgw_manager_event_release_call:
            return on_manager_release_call(manager_message->handle, &manager_message->body.call_info);
        break;
        default:;
    }

    return vgw_failed;
}

// обработчик события открытия нового стрима (см. структуру vgw_stream_info_t)
static vgw_result_t on_call_open_stream(vgw_handle_t handle
                                        , struct vgw_stream_info_t* stream_info)
{
    return vgw_ok;
}

// обработчик события закрытия ранее открытого стрима
static vgw_result_t on_call_close_stream(vgw_handle_t handle
                                         , struct vgw_stream_info_t* stream_info)
{
    return vgw_ok;
}

// обработчик запроса на чтение медиафрейма. Тут поподробнее!
// Тут нету api типа послать фрейм на передачу и забыть о нем!
// Тут именно передаем фрейм в обработчике библиотеки. Это обусловлено тем
// что мы заранее не знает сырой формат который нужен стеку передачи.
// Настройки транскодеров и конвертеров внутри ядра исполняющей
// библиотеки могут поменяться в процессе звонка! Поэтому мы не сами ложим
// фрейм на передачу, а готовим его заранее и жем когда сама библиотека нас
// попросит. Дескриптор звонка мы знаем, id потока тоже, так что с идентификацией
// проблем нет. Для аудио фреймов: формат, частота и каналы должны совпедать,
// поэтому в основной логике приложения нужно ресемплировать фрейм в обработчике
// до возврата. Буфер уже выделен библиотекой, надо его только заполить по указателю
// buffer_ptr. В случае audio, заполнять надо тем же размером что и buffer_size,
// его же и возвращать. Для видео, размер фрейма может быть меньше чем buffer_ptr,
// но формат должен совпадать с запрашиваемым. Обычно это YUV420P. Ширина и высота,
// могут быть любые, но возвращаемый размер должен соответсвовать им.
// !!!И еще самое важное!!! Приложение должно само управлять интенсивностью чтения
// фреймов! Т.е. при возврате учитывать такие вещи как frame_rate. Как? Обычной
// задержкой. Библиотека не знает частоту кадров источника, поэтому она выполняет
// просто поллинг чтения через этот вызов. Например если мы передаем по 20 мс семплы аудио,
// то грубо на каждом вызове нам надо сделать задержку в 20 мс до возврата значения.
// Если например мы хотим передать 30 FPS видео поток, то получается что на каждом
// фрейме надо сделать задержку в 1000/30 мсек. Пока так. Есть мысли как упростить
// эту тему, но не на этой фазе разработки.

// В примере используется встроенная очередь, которая учитывает все эти особенности.
// Возврат требут размер прочитанных данных!
static vgw_result_t on_call_read_frame(vgw_handle_t handle
                                       , struct vgw_media_frame_t* media_frame)
{
    if (call_handle == handle)
    {
        switch(media_frame->media_type)
        {
            case vgw_media_type_audio:
            {
                // если очереди открыты
                if (__vgw_is_result_ok(audio_queue))
                {
                    // пытаемся считать фрейм с таймаутом (таймаут обеспечит
                    // необходимую синхронизацию). Если чтение успешно
                    // то возврат будет равен размеру заполненных данных фрейма.
                    return vgw_pop_frame(audio_queue
                                         , media_frame
                                         , __default_read_queue_timeout_ms);
                }
            }
            break;
            case vgw_media_type_video:
            {
                if (__vgw_is_result_ok(video_queue))
                {
                    return vgw_pop_frame(video_queue
                                         , media_frame
                                         , __default_read_queue_timeout_ms);
                }
            }
            break;
            default:;
        }
    }

    return vgw_failed;
}

// обработчик приема входящего фрейма от удаленной стороны. Тут все проще, что хотим
// то и делаем с фреймами. Синхронизации вызова как для on_call_read_frame не требуется.
// Возврат требут размер записанных данных!
static vgw_result_t on_call_write_frame(vgw_handle_t handle
                                        , struct vgw_media_frame_t* media_frame)
{
    if (call_handle == handle)
    {
        switch(media_frame->media_type)
        {
            case vgw_media_type_audio:
            {
                // если очереди открыты
                if (__vgw_is_result_ok(audio_queue))
                {
                    // записываем фрейм в очередь. Возвращаемый размер
                    // будет равен размеру данных фрейма
                    return vgw_push_frame(audio_queue
                                         , media_frame);
                }
            }
            break;
            case vgw_media_type_video:
            {
                if (__vgw_is_result_ok(video_queue))
                {
                    return vgw_push_frame(video_queue
                                          , media_frame);
                }
            }
            break;
            default:;
        }
    }

    return vgw_failed;
}
// обработчик события приема DTMF-сигналов
static vgw_result_t on_call_user_input(vgw_handle_t handle
                                       , const char* tones)
{
    if (call_handle == handle)
    {
        // заворачиваем DTMF тупо обратно
        vgw_send_user_input(handle
                            , tones);
        return vgw_ok;
    }

    return vgw_failed;
}

// диспетчеризация событий звонков
static vgw_result_t on_call_message(struct vgw_call_message_t* call_message)
{
    switch(call_message->event_type)
    {
        case vgw_call_event_open_stream:
            return on_call_open_stream(call_message->handle, &call_message->body.stream_info);
        break;
        case vgw_call_event_close_stream:
            return on_call_close_stream(call_message->handle, &call_message->body.stream_info);
        break;
        case vgw_call_event_read_frame:
            return on_call_read_frame(call_message->handle, &call_message->body.media_frame);
        break;
        case vgw_call_event_write_frame:
            return on_call_write_frame(call_message->handle, &call_message->body.media_frame);
        break;
        case vgw_call_event_user_input:
            return on_call_user_input(call_message->handle, call_message->body.tones);
        break;
        default:;
    }

    return vgw_failed;
}

// диспетчеризация событий voip
static vgw_result_t callback(enum vgw_message_type_t type
                             , void* message)
{
    switch(type)
    {
        case vgw_message_type_manager:
            return on_manager_message((struct vgw_manager_message_t*)message);
        break;
        case vgw_message_type_call:
            return on_call_message((struct vgw_call_message_t*)message);
        break;
        default:;
    }

    return vgw_failed;
}

int c_app(int argc
          , const char *argv[])
{
    // настройки можно вообще не заполнять (но обнулить), тогда они будут по умолчанию.
    struct vgw_call_manager_config_t manager_config = {};
    manager_config.user_name = "user";
    manager_config.display_name = "admin";
    manager_config.max_bitrate = 2048000;
    manager_config.min_rtp_port = 10000;
    manager_config.max_rtp_port = 11000;
    manager_config.max_rtp_packet_size = 1400;
    manager_config.audio_codecs = "G722;PCMA;PCMU";
    manager_config.video_codecs = "H264;H265;H263-1998";

    vgw_handle_t manager = vgw_create_manager(&manager_config
                                              , callback);

    if (__vgw_is_result_ok(manager))
    {
        audio_queue = vgw_create_frame_queue(__defult_queue_size);
        video_queue = vgw_create_frame_queue(__defult_queue_size);

        if (__vgw_is_result_ok(vgw_start_manager(manager)))
        {

            if (argc == 2)
            {
                vgw_make_call(manager
                              , argv[1]);
            }

            while(!stop)
            {
                usleep(100000);
            }

            vgw_stop_manager(manager);
        }

        vgw_release_frame_queue(audio_queue);
        vgw_release_frame_queue(video_queue);

        vgw_release_manager(manager);
    }

    return 0;
}
