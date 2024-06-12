#pragma once

#include "error.h"

#include "pproto/commands/base.h"
#include "pproto/commands/paging.h"
#include "pproto/commands/time_range.h"

namespace pproto {
namespace command {

//----------------------------- Список команд --------------------------------

/**
  Вспомогательная команда, используется для коммуникации внутри приложения
*/
extern const QUuidEx EventInternal;

/**
  Получение списка событий
*/
extern const QUuidEx EventLog;

/**
  Запрос на получение видео-лог записи
*/
extern const QUuidEx VideoEventGet;

/**
  Команда для пересылки чанка видео-лог записи
*/
extern const QUuidEx VideoEventChunk;

/**
  Команда сигнализирует об окончании пересылки видео-лог записи
*/
extern const QUuidEx VideoEventFinish;

/**
  Команда сигнализирует о неудаче при пересылке записи
*/
extern const QUuidEx VideoEventError;

} // namespace command

//---------------- Структуры данных используемые в сообщениях ----------------

namespace data {

struct RelayOpen
{
    QDateTime dtime;
    bool isManually = {false};

    DECLARE_B_SERIALIZE_FUNC

    J_SERIALIZE_BEGIN
        J_SERIALIZE_ITEM( dtime      )
        J_SERIALIZE_ITEM( isManually )
    J_SERIALIZE_END
};

struct EventItem
{
    // Идентификатор события
    QUuidEx id;

    // Время начала события
    QDateTime vialDetect;

    // Время окончания события
    QDateTime vialLost;

    bool isDefected = {false};

    // Достоверность дефекта пузырька (в процентах)
    qint32 percentDefect = {0};

    // Достоверность детектирования пузырька (в процентах)
    qint32 percentDetection = {0};

    QUuidEx saverId;

    DECLARE_B_SERIALIZE_FUNC

    J_SERIALIZE_BEGIN
        J_SERIALIZE_ITEM( id               )
        J_SERIALIZE_ITEM( vialDetect       )
        J_SERIALIZE_ITEM( vialLost         )
        J_SERIALIZE_ITEM( isDefected       )
        J_SERIALIZE_ITEM( percentDefect    )
        J_SERIALIZE_ITEM( percentDetection )
        J_SERIALIZE_ITEM( saverId          )
    J_SERIALIZE_END
};

struct EventInternal : EventItem, Data<&command::EventInternal,
                                        Message::Type::Command>
{};

struct EventFilter
{
    // Флаг определяет какие пузырьки возвращать
    //   0 - все пузырьки
    //   1 - дефектные
    //   2 - не дефектные
    qint32 vialState = {0};

    DECLARE_B_SERIALIZE_FUNC

    J_SERIALIZE_BEGIN
        J_SERIALIZE_ITEM( vialState )
    J_SERIALIZE_END
};

struct EventLog : Data<&command::EventLog,
                        Message::Type::Command,
                        Message::Type::Answer>
{
    // Фильтр выбора данных
    EventFilter filter;

    // Временной диапазон выборки
    TimeRange timeRange;

    // Фильтр для постраничной выборки данных
    PagingInfo paging;

    // Результирующая выборка
    QVector<EventItem> items;

    // Общее количество пузырьков в выборке
    qint32 countAll = {0};

    // Количество дефектных пузырьков в выборке
    qint32 countDefect = {0};

    //bool toFile = {false};

    DECLARE_B_SERIALIZE_FUNC

    J_SERIALIZE_BEGIN
        J_SERIALIZE_ITEM( filter      )
        J_SERIALIZE_ITEM( timeRange   )
        J_SERIALIZE_ITEM( paging      )
        J_SERIALIZE_ITEM( items       )
        J_SERIALIZE_ITEM( countAll    )
        J_SERIALIZE_ITEM( countDefect )
        //J_SERIALIZE_ITEM( toFile )
    J_SERIALIZE_END
};

struct VideoEventGet : Data<&command::VideoEventGet,
                             Message::Type::Command,
                             Message::Type::Answer>
{
    // Идентификатор события
    QUuidEx eventId;

    // Количество чанков (возвращается в ответе)
    qint32 chunkCount = {0};

    DECLARE_B_SERIALIZE_FUNC
};

struct VideoEventChunk : Data<&command::VideoEventChunk,
                               Message::Type::Command>
{
    // Идентификатор события
    QUuidEx eventId;

    // Индекс чанка
    qint32 index = {0};

    // Контрольная сумма чанка
    quint32 crc = {0};

    // Данные чанка
    SByteArray data;

    DECLARE_B_SERIALIZE_FUNC
};

struct VideoEventFinish : Data<&command::VideoEventFinish,
                                Message::Type::Command>
{
    // Идентификатор события
    QUuidEx eventId;

    // Контрольная сумма видео-лог записи
    quint32 crc = {0};

    DECLARE_B_SERIALIZE_FUNC
};

struct VideoEventError : Data<&command::VideoEventError,
                               Message::Type::Command>
{
    // Идентификатор события
    QUuidEx eventId;
    ErrorInfo errorInfo;

    DECLARE_B_SERIALIZE_FUNC
};

} // namespace data
} // namespace pproto
