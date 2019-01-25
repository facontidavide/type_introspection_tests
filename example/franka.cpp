#include <ros_introspection_test/FrankaError.h>
#include <ros_type_introspection/ros_introspection.hpp>


void Register(RosIntrospection::Parser* parser,
              const char* topic_name,
              const char* data_type,
              const char* definition)
{
    using namespace RosIntrospection;

    // add this type to the parser.
    parser->registerMessageDefinition(
                topic_name,
                ROSType(data_type),
                definition);
}

void DeserializeAndPrint(RosIntrospection::Parser* parser,
                         const std::string& topic_name,
                         std::vector<uint8_t> & buffer)
{

    using namespace RosIntrospection;
    FlatMessage flat_container;
    parser->deserializeIntoFlatContainer( topic_name,
                                          absl::Span<uint8_t>(buffer),
                                          &flat_container, 100 );

    for(auto& it: flat_container.value)
    {
        bool value = it.second.convert<int8_t>();

        if( value )
        {
            std::cout << it.first.toStdString() << " : "
                      << it.second.convert<double>() << std::endl;
        }
    }
}

template <typename Message>
std::vector<uint8_t> getSerializedMessage(const Message& message)
{
    size_t msg_length = ros::serialization::serializationLength(message);
    std::vector<uint8_t> buffer(msg_length);
    ros::serialization::OStream stream(buffer.data(), buffer.size());
    ros::serialization::serialize(stream, message);
    return buffer;
}

int main(int argc, char **argv)
{
    using ros_introspection_test::FrankaError;

    RosIntrospection::Parser parser;

    // do this only once
    Register(&parser, "error",
             ros::message_traits::DataType<FrankaError>::value(),
             ros::message_traits::Definition<FrankaError>::value());


    FrankaError sample{};
    sample.cartesian_reflex = true;
    sample.force_control_safety_violation = true;

    // usually you get this serialized message from topic_tools::ShapeShifter or rosbag::Message
    std::vector<uint8_t> serialized_msg = getSerializedMessage(sample);

    DeserializeAndPrint(&parser, "error", serialized_msg);
    return 0;
}

