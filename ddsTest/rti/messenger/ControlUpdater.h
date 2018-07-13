#ifndef CONTROLUPDATER_H
#define CONTROLUPDATER_H

#include "ControlVariable.h"
#include <dds/sub/Subscriber.hpp>
#include <dds/sub/DataReader.hpp>
#include <mutex>

//=====================================================================================================================
/// Listen for control commands from monitor
/// Message is a vector<uint8_t> that can be typecast to scalar T in the enclosing ControlUpdaterT<T>
class ControlUpdateListener :  public dds::sub::NoOpDataReaderListener<dds::core::KeyedBytesTopicType>
//=====================================================================================================================
{
public:
    ControlUpdateListener();
    ~ControlUpdateListener();
    void on_data_available(dds::sub::DataReader<dds::core::KeyedBytesTopicType>& reader) final;
    bool isUpdated() const;
    const dds::core::KeyedBytesTopicType& data() const;
private:
    mutable std::mutex              _mutex;
    mutable bool                    _isUpdated;
    dds::core::KeyedBytesTopicType  _keyedBytes;
};


//=====================================================================================================================
class AbstractControlUpdater
//=====================================================================================================================
{
public:
    virtual std::string name() const = 0;
    virtual std::vector<uint8_t> toBytes() const = 0;
    virtual bool updatePlant() = 0;
    virtual ~AbstractControlUpdater() = default;
};

//=====================================================================================================================
template <typename T>
class ControlUpdaterT : public AbstractControlUpdater
//=====================================================================================================================
{
public:
    ControlUpdaterT(dds::domain::DomainParticipant& participant, dds::sub::Subscriber& subscriber,
                          const std::string& topicName,
                          T* pVariable, const T& min, const T& max)
        : _topicName(topicName),
          _reader(subscriber, dds::topic::Topic<dds::core::KeyedBytesTopicType>(participant, _topicName)),
          _variable(*pVariable, min, max),
          _pNakedPointer(pVariable)
    {
        _reader.listener(&_listener, dds::core::status::StatusMask::data_available());
    }

    ~ControlUpdaterT()
    {
        _reader.close();
        std::cout << "[~ControlUpdaterT (" << _topicName << ")]" << std::endl;
    }

    bool updatePlant() final
    {
        bool isUpdated = _listener.isUpdated();
        if( isUpdated )
        {
            // get the last received setting
            const dds::core::KeyedBytesTopicType& data = _listener.data();

            // check keys match
            const std::string key = data.key().to_std_string();
            isUpdated = (key == _topicName);
            if( isUpdated )
            {
                // convert data to value type and set limits
                const std::vector<uint8_t> vec = data.value();
                const T* pValue = reinterpret_cast<const T*>(&vec[0]);
                _variable.value(*pValue);

                // apply to plant
                *_pNakedPointer = _variable.value();
                //std::cout << "[ControlUpdaterT (" << _topicName << ")]: set " << *pValue << std::endl;
            }
            else
            {
                std::cout << "[PlantControlVariableT (" << _topicName << ")]: ERROR - key mismatch. Got (" << key << ")" << std::endl;
            }
        }
        return isUpdated;
    }

    std::vector<uint8_t> toBytes() const final
    {
        return _variable.toBytes();
    }

    std::string name() const final { return _topicName; }


private:
    const std::string   _topicName;
    dds::sub::DataReader<dds::core::KeyedBytesTopicType> _reader;
    ControlUpdateListener  _listener;
    ControlVariableT<T> _variable;
    T* _pNakedPointer;
};


#endif // CONTROLUPDATER_H
