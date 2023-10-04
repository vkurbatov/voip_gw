#ifndef VOIP_API_H
#define VOIP_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t vgw_handle_t;
typedef int32_t vgw_result_t;

#define vgw_ok (0)
#define vgw_failed (-1)

#define __vgw_is_result_ok(result) ((result) >= 0)
#define __vgw_is_result_failed(result) ((result) < 0)

#pragma pack(push, 1)

// Категория сообщения. Пока категорий всего две:
enum vgw_message_type_t
{
    vgw_message_type_manager = 0,                               // сообщения от менеджера звонков
    vgw_message_type_call                                       // сообщения от звонка
};

// Медиа типы
enum vgw_media_type_t
{
    vgw_media_type_undefined = 0,
    vgw_media_type_audio,
    vgw_media_type_video,
    vgw_media_type_application
};

// Тип медиа-стрима, по сути указывает направление потока
enum vgw_stream_type_t
{
    vgw_stream_type_inbound = 0,
    vgw_stream_type_outbound
};

enum vgw_call_type_t                                            // тип звонка, по сути указывает его направление
{
    vgw_call_type_undefined = 0,
    vgw_call_type_incoming,
    vgw_call_type_outgoing
};

struct vgw_call_manager_config_t                                // конфигурация менеджера звонков. Задается только при создании менеджера
                                                                // значения 0/NULL указывают что настройки будут испольованы по умолчанию
{
    const char*                         user_name;              // имя ползователя, фигурируемое в адресе.
    const char*                         display_name;           // имя фигурируемое в заголовках.
    uint32_t                            max_bitrate;            // максимальный битрейт потоков (пока не работает).
    uint16_t                            min_rtp_port;           // минимальный..
    uint16_t                            max_rtp_port;           // и максимальный порты для RTP
    uint16_t                            max_rtp_packet_size;    // максимальный размер RTP-пакетов (для mtu)
    const char*                         audio_codecs;           // список исползьуемых аудио-кодеков (через ';'). NULL - будут использованы все кодеки;
    const char*                         video_codecs;           // список исползьуемых видео-кодеков (через ';'). NULL - будут использованы все кодеки;
};

struct vgw_stream_info_t                                        // cтруктура описания медиа-стримов
{
    enum vgw_media_type_t               media_type;
    enum vgw_stream_type_t              stream_type;
    int32_t                             stream_id;              // порядковый номер стрима (начиная с 1). Обычно 1 - аудио, 2 - видео
};

struct vgw_call_info_t
{
    vgw_handle_t                        call_handle;            // дескриптор двонка
    enum vgw_call_type_t                call_type;              // входящий или исходящий
    const char*                         remote_url;             // url удаленного абонента
    const char*                         call_id;                // уникальный идентификатор звонка
};


enum vgw_manager_event_type_t                                   // типы событий менежера
{
    vgw_manager_event_started = 0,                              // менеджер запущен
    vgw_manager_event_stopped,                                  // менеджер остановлен
    vgw_manager_event_create_call,                              // новый звонок (входящий или исходящий)
    vgw_manager_event_release_call                              // отбит звонок
};

struct vgw_manager_message_t                                    // структура сообщений менеджера
{
    enum vgw_manager_event_type_t       event_type;
    vgw_handle_t                        handle;                 // дескриптор менеджера
    union
    {
        struct vgw_call_info_t          call_info;              // только для create_call, release_call
    }                                   body;                   // тело сообщения
};

struct vgw_audio_info_t                                         // структура описания формата
{
    const char*                         format;                 // имя формата (задается системой, обычно PCM-16-***)
    uint32_t                            sample_rate;            // частота дескритизации 8000, 16000, 32000, 48000
    uint32_t                            channels;               // количество каналов 1-2
};

struct vgw_video_info_t                                         // структура описания формата
{
    const char*                         format;                 // имя формата (задается системой, обычно YUV420P)
    uint32_t                            width;                  // ширина фрейма
    uint32_t                            height;                 // высота фрейма
};

struct vgw_media_frame_t                                        // струткура описания медиа-фрейма
{
    int32_t                             stream_id;              // идентификатор потока (не испольуется в очереди фреймов)
    enum vgw_media_type_t               media_type;

    union
    {
        struct vgw_audio_info_t         audio_info;             // только для аудио-типа
        struct vgw_video_info_t         video_info;             // только для видео-типа
    }                                   info;

    uint32_t                            buffer_size;            // размер буфера данных
    void*                               buffer_ptr;             // указатель на буфер данных (ссылочный)
};

enum vgw_call_event_type_t                                      // типы событий звонка
{
    vgw_call_event_open_stream = 0,                             // открыт новый стрим
    vgw_call_event_close_stream,                                // закрыт действующий стрим
    vgw_call_event_read_frame,                                  // запрос на чтение медиа-фрейма для передачи удаленной стороне
    vgw_call_event_write_frame,                                 // запрос на запись медиа-фрейма принятого от удаленной стороны
    vgw_call_event_user_input                                   // событие тонального набора от удаленной станции
};

struct vgw_call_message_t
{
   enum vgw_call_event_type_t           event_type;
    vgw_handle_t                        handle;                 // дескриптор звонка
    union
    {
        struct vgw_stream_info_t        stream_info;            // open_stream, close_stream
        struct vgw_media_frame_t        media_frame;            // read_frame, write_frame
        const char*                     tones;                  // user_input
    }                                   body;
};

#pragma pack(pop)

// функция обратного вызова для обработки voip-событий
typedef vgw_result_t (*message_callback_t)(enum vgw_message_type_t type
                                           , void* message);

// запрос на создание менеджера звонков (может быть только один экземпляр)
// возвращает дескриптор менеджера
vgw_handle_t vgw_create_manager(const struct vgw_call_manager_config_t* manager_config
                                , message_callback_t message_callback);

// освободить менеджер звонков
vgw_result_t vgw_release_manager(vgw_handle_t manager_handle);

// запустить менеджер звонков
vgw_result_t vgw_start_manager(vgw_handle_t manager_handle);

// остановить менеджер звонков
vgw_result_t vgw_stop_manager(vgw_handle_t manager_handle);

// создать исходящий звонок (например, url=sip:user@192.168.0.1:3060)
vgw_result_t vgw_make_call(vgw_handle_t manager_handle
                           , const char* url);

// отправить DTMF-последовательность удаленной стороне
vgw_result_t vgw_send_user_input(vgw_handle_t call_handle
                                  , const char* tones);

// отбить звонок
vgw_result_t vgw_hangup_call(vgw_handle_t call_handle);

// создать очредь фреймов, queue_size - максимально количество фреймов в очереди
// возвращает дескриптор очереди
vgw_handle_t vgw_create_frame_queue(uint32_t queue_size);

// удалить очередь
vgw_handle_t vgw_release_frame_queue(vgw_handle_t handle);

// очистить очередь
vgw_handle_t vgw_clear_frame_queue(vgw_handle_t handle);

// записать в очередь фрейм
vgw_result_t vgw_push_frame(vgw_handle_t handle
                            , const struct vgw_media_frame_t* media_frame);

// считать из очереди медиафрейм, timeout_ms - таймаут ожидания если очередь пустая
vgw_result_t vgw_pop_frame(vgw_handle_t handle
                           , struct vgw_media_frame_t* media_frame
                           , uint32_t timeout_ms);


#ifdef __cplusplus
}
#endif


#endif // VOIP_API_H
