#include "ControlUpdater.h"

//=====================================================================================================================
ControlUpdateListener::ControlUpdateListener()
//=====================================================================================================================
    : _isUpdated(false),
      _keyedBytes()
{}

//---------------------------------------------------------------------------------------------------------------------
ControlUpdateListener::~ControlUpdateListener()
//---------------------------------------------------------------------------------------------------------------------
{
    //std::cout << "~[ControlUpdateListener]" << std::endl;
}

//---------------------------------------------------------------------------------------------------------------------
void ControlUpdateListener::on_data_available(dds::sub::DataReader<dds::core::KeyedBytesTopicType>& reader)
//---------------------------------------------------------------------------------------------------------------------
{
    dds::sub::LoanedSamples<dds::core::KeyedBytesTopicType> samples = reader.take();
    for (const auto& sample : samples)
    {
        if (sample.info().valid())
        {
            const dds::core::KeyedBytesTopicType& data = static_cast<const dds::core::KeyedBytesTopicType&>(sample.data());
            const std::string variableName = data.key().to_std_string();
            //std::cout << "[ControlUpdateListener (" << variableName << ")]: received bytes " << std::endl;

            std::lock_guard<std::mutex> guard(_mutex);
            _keyedBytes.key(variableName);
            _keyedBytes.value(data.value());
            _isUpdated = true;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool ControlUpdateListener::isUpdated() const
//---------------------------------------------------------------------------------------------------------------------
{
    std::lock_guard<std::mutex> guard(_mutex);
    return _isUpdated;
}

//---------------------------------------------------------------------------------------------------------------------
const dds::core::KeyedBytesTopicType& ControlUpdateListener::data() const
//---------------------------------------------------------------------------------------------------------------------
{
    std::lock_guard<std::mutex> guard(_mutex);
    _isUpdated = false;
    return _keyedBytes;
}


