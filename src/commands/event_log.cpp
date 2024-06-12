#include "event_log.h"
#include "pproto/commands/pool.h"

namespace pproto {
namespace command {

#define REGISTRY_COMMAND_SINGLPROC(COMMAND, UUID) \
    const QUuidEx COMMAND = command::Pool::Registry{UUID, #COMMAND, false};

REGISTRY_COMMAND_SINGLPROC(EventInternal,     "3398c1d4-a52c-46ec-99f4-433abd42f3e6")
REGISTRY_COMMAND_SINGLPROC(EventLog,          "82a1ac8a-09fe-4c4b-b152-ac9a60be6e7d")
REGISTRY_COMMAND_SINGLPROC(VideoEventGet,     "99ab5e3d-48af-43b9-b555-63969f753d35")
REGISTRY_COMMAND_SINGLPROC(VideoEventChunk,   "d2558b9a-5299-40fd-b49d-c40077bb934e")
REGISTRY_COMMAND_SINGLPROC(VideoEventFinish,  "9f526f0c-4bf1-4f08-82c2-1da979a28087")
REGISTRY_COMMAND_SINGLPROC(VideoEventError,   "1678d9d3-c6a5-410b-b9e1-2de363c54008")

#undef REGISTRY_COMMAND_SINGLPROC

} // namespace command

namespace data {

bserial::RawVector RelayOpen::toRaw() const
{
    B_SERIALIZE_V1(stream)
    stream << dtime;
    stream << isManually;
    B_SERIALIZE_RETURN
}

void RelayOpen::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)
    stream >> dtime;
    stream >> isManually;
    B_DESERIALIZE_END
}

bserial::RawVector EventItem::toRaw() const
{
    B_SERIALIZE_V1(stream)
    stream << id;
    stream << vialDetect;
    stream << vialLost;
    stream << isDefected;
    stream << percentDefect;
    stream << percentDetection;
    stream << saverId;
    B_SERIALIZE_RETURN
}

void EventItem::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)
    stream >> id;
    stream >> vialDetect;
    stream >> vialLost;
    stream >> isDefected;
    stream >> percentDefect;
    stream >> percentDetection;
    stream >> saverId;
    B_DESERIALIZE_END
}

bserial::RawVector EventFilter::toRaw() const
{
    B_SERIALIZE_V1(stream)
    stream << vialState;
    B_SERIALIZE_RETURN
}

void EventFilter::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)
    stream >> vialState;
    B_DESERIALIZE_END
}

bserial::RawVector EventLog::toRaw() const
{
    B_SERIALIZE_V1(stream)
    stream << filter;
    stream << timeRange;
    stream << paging;
    stream << items;
    stream << countAll;
    stream << countDefect;
    //stream << toFile;
    B_SERIALIZE_RETURN
}

void EventLog::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)
    stream >> filter;
    stream >> timeRange;
    stream >> paging;
    stream >> items;
    stream >> countAll;
    stream >> countDefect;
    //stream >> toFile;
    B_DESERIALIZE_END
}

bserial::RawVector VideoEventGet::toRaw() const
{
    B_SERIALIZE_V1(stream)
    stream << eventId;
    stream << chunkCount;
    B_SERIALIZE_RETURN
}

void VideoEventGet::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)
    stream >> eventId;
    stream >> chunkCount;
    B_DESERIALIZE_END
}

bserial::RawVector VideoEventChunk::toRaw() const
{
    B_SERIALIZE_V1(stream)
    stream << eventId;
    stream << index;
    stream << crc;
    stream << data;
    B_SERIALIZE_RETURN
}

void VideoEventChunk::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)
    stream >> eventId;
    stream >> index;
    stream >> crc;
    stream >> data;
    B_DESERIALIZE_END
}

bserial::RawVector VideoEventFinish::toRaw() const
{
    B_SERIALIZE_V1(stream)
    stream << eventId;
    stream << crc;
    B_SERIALIZE_RETURN
}

void VideoEventFinish::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)
    stream >> eventId;
    stream >> crc;
    B_DESERIALIZE_END
}

bserial::RawVector VideoEventError::toRaw() const
{
    B_SERIALIZE_V1(stream)
    stream << eventId;
    stream << errorInfo;
    B_SERIALIZE_RETURN
}

void VideoEventError::fromRaw(const bserial::RawVector& vect)
{
    B_DESERIALIZE_V1(vect, stream)
    stream >> eventId;
    stream >> errorInfo;
    B_DESERIALIZE_END
}

} // namespace data
} // namespace pproto
