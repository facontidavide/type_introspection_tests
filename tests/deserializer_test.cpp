#include "config.h"
#include <gtest/gtest.h>

#include "ros_type_introspection/ros_introspection.hpp"
#include <sensor_msgs/JointState.h>
#include <sensor_msgs/NavSatStatus.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/Image.h>
#include <std_msgs/Int16MultiArray.h>

using namespace ros::message_traits;
using namespace RosIntrospection;


TEST(Deserialize, JointState)

{
  RosIntrospection::Parser parser;

  parser.registerMessageDefinition(
        "JointState",
        ROSType(DataType<sensor_msgs::JointState>::value()),
        Definition<sensor_msgs::JointState>::value());

  sensor_msgs::JointState joint_state;

  const int NUM = 15;

  joint_state.header.seq = 2016;
  joint_state.header.stamp.sec  = 1234;
  joint_state.header.stamp.nsec = 567*1000*1000;
  joint_state.header.frame_id = "pippo";

  joint_state.name.resize( NUM );
  joint_state.position.resize( NUM );
  joint_state.velocity.resize( NUM );
  joint_state.effort.resize( NUM );

  std::string names[NUM];
  names[0] = ("hola");
  names[1] = ("ciao");
  names[2] = ("bye");

  for (int i=0; i<NUM; i++)
  {
    joint_state.name[i] = names[i%3];
    joint_state.position[i]= 10+i;
    joint_state.velocity[i]= 30+i;
    joint_state.effort[i]= 50+i;
  }

  std::vector<uint8_t> buffer( ros::serialization::serializationLength(joint_state) );
  ros::serialization::OStream stream(buffer.data(), buffer.size());
  ros::serialization::Serializer<sensor_msgs::JointState>::write(stream, joint_state);

  FlatMessage flat_container;
  parser.deserializeIntoFlatContainer("JointState",  absl::Span<uint8_t>(buffer),  &flat_container,100);

  if(VERBOSE_TEST){
    for(auto&it: flat_container.value) {
      std::cout << it.first << " >> " << it.second.convert<double>() << std::endl;
    }

    for(auto&it: flat_container.name) {
      std::cout << it.first << " >> " << it.second << std::endl;
    }
  }

  EXPECT_EQ( flat_container.value[0].first.toStdString() , ("JointState/header/seq"));
  EXPECT_EQ( flat_container.value[0].second.convert<int>(), 2016 );
  EXPECT_EQ( flat_container.value[1].first.toStdString() , ("JointState/header/stamp"));
  EXPECT_EQ( flat_container.value[1].second.convert<double>(),   double(1234.567)  );
  EXPECT_EQ( flat_container.value[1].second.convert<ros::Time>(), joint_state.header.stamp  );

  EXPECT_EQ( flat_container.value[2].first.toStdString() , ("JointState/position.0"));
  EXPECT_EQ( flat_container.value[2].second.convert<int>(), 10 );
  EXPECT_EQ( flat_container.value[3].first.toStdString() , ("JointState/position.1"));
  EXPECT_EQ( flat_container.value[3].second.convert<int>(), 11 );
  EXPECT_EQ( flat_container.value[4].first.toStdString() , ("JointState/position.2"));
  EXPECT_EQ( flat_container.value[4].second.convert<int>(), 12 );
  EXPECT_EQ( flat_container.value[16].first.toStdString() , ("JointState/position.14"));
  EXPECT_EQ( flat_container.value[16].second.convert<int>(), 24 );

  EXPECT_EQ( flat_container.value[17].first.toStdString() , ("JointState/velocity.0"));
  EXPECT_EQ( flat_container.value[17].second.convert<int>(), 30 );
  EXPECT_EQ( flat_container.value[18].first.toStdString() , ("JointState/velocity.1"));
  EXPECT_EQ( flat_container.value[18].second.convert<int>(), 31 );
  EXPECT_EQ( flat_container.value[19].first.toStdString() , ("JointState/velocity.2"));
  EXPECT_EQ( flat_container.value[19].second.convert<int>(), 32 );
  EXPECT_EQ( flat_container.value[31].first.toStdString() , ("JointState/velocity.14"));
  EXPECT_EQ( flat_container.value[31].second.convert<int>(), 44 );

  EXPECT_EQ( flat_container.value[32].first.toStdString() , ("JointState/effort.0"));
  EXPECT_EQ( flat_container.value[32].second.convert<int>(), 50 );
  EXPECT_EQ( flat_container.value[33].first.toStdString() , ("JointState/effort.1"));
  EXPECT_EQ( flat_container.value[33].second.convert<int>(), 51 );
  EXPECT_EQ( flat_container.value[34].first.toStdString() , ("JointState/effort.2"));
  EXPECT_EQ( flat_container.value[34].second.convert<int>(), 52 );
  EXPECT_EQ( flat_container.value[46].first.toStdString() , ("JointState/effort.14"));
  EXPECT_EQ( flat_container.value[46].second.convert<int>(), 64 );

  EXPECT_EQ( flat_container.name[0].first.toStdString() , ("JointState/header/frame_id"));
  EXPECT_EQ( flat_container.name[0].second, ("pippo") );

  EXPECT_EQ( flat_container.name[1].first.toStdString() , ("JointState/name.0"));
  EXPECT_EQ( flat_container.name[1].second, ("hola") );
  EXPECT_EQ( flat_container.name[2].first.toStdString() , ("JointState/name.1"));
  EXPECT_EQ( flat_container.name[2].second, ("ciao") );
  EXPECT_EQ( flat_container.name[3].first.toStdString() , ("JointState/name.2"));
  EXPECT_EQ( flat_container.name[3].second, ("bye") );

  //---------------------------------
  std::vector<std_msgs::Header> headers;

  Parser::VisitingCallback callbackReadAndStore = [&headers](const ROSType&, absl::Span<uint8_t>& raw_data)
  {
    std_msgs::Header msg;
    ros::serialization::IStream s( raw_data.data(), raw_data.size() );
    ros::serialization::deserialize(s, msg);
    headers.push_back( std::move(msg) );
  };

  Parser::VisitingCallback callbackOverwiteInPlace = [&headers](const ROSType&, absl::Span<uint8_t>& raw_data)
  {
    std_msgs::Header msg;
    ros::serialization::IStream is( raw_data.data(), raw_data.size() );
    ros::serialization::deserialize(is, msg);

    ASSERT_EQ(ros::serialization::serializationLength(msg), raw_data.size());

    // msg.frame_id = "here";  //NOTE: I can NOT change the size of an array, nor a string
    msg.seq = 666;
    msg.stamp.sec = 1;
    msg.stamp.nsec = 2;

    ros::serialization::OStream os( raw_data.data(), raw_data.size() );
    ros::serialization::serialize(os, msg);

    ASSERT_EQ(ros::serialization::serializationLength(msg), raw_data.size());
  };


  absl::Span<uint8_t> buffer_view(buffer);
  const ROSType header_type( DataType<std_msgs::Header>::value() );

  parser.applyVisitorToBuffer( "JointState", header_type,
                               buffer_view, callbackReadAndStore);

  EXPECT_EQ(headers.size(), 1);
  const std_msgs::Header& header = headers[0];
  EXPECT_EQ(header.seq,        joint_state.header.seq);
  EXPECT_EQ(header.stamp.sec,  joint_state.header.stamp.sec);
  EXPECT_EQ(header.stamp.nsec, joint_state.header.stamp.nsec);
  EXPECT_EQ(header.frame_id,   joint_state.header.frame_id);
  //--------------------------------------------
  parser.applyVisitorToBuffer( "JointState", header_type,
                               buffer_view, callbackOverwiteInPlace);

  parser.applyVisitorToBuffer( "JointState", header_type,
                               buffer_view, callbackReadAndStore);

  EXPECT_EQ(headers.size(), 2);
  const std_msgs::Header& header_mutated = headers[1];
  EXPECT_EQ(header_mutated.seq,        666);
  EXPECT_EQ(header_mutated.stamp.sec,  1);
  EXPECT_EQ(header_mutated.stamp.nsec, 2);

}

TEST( Deserialize, NavSatStatus)

{
  // We test this because we want to test that constant fields are skipped.
  RosIntrospection::Parser parser;

  parser.registerMessageDefinition(
        "nav_stat",
        ROSType(DataType<sensor_msgs::NavSatStatus>::value()),
        Definition<sensor_msgs::NavSatStatus>::value());

  sensor_msgs::NavSatStatus nav_stat;
  nav_stat.status  = nav_stat.STATUS_GBAS_FIX;  // 2
  nav_stat.service = nav_stat.SERVICE_COMPASS; // 4


  std::vector<uint8_t> buffer( ros::serialization::serializationLength(nav_stat) );
  ros::serialization::OStream stream(buffer.data(), buffer.size());
  ros::serialization::Serializer<sensor_msgs::NavSatStatus>::write(stream, nav_stat);

  FlatMessage flat_container;
  parser.deserializeIntoFlatContainer("nav_stat",  absl::Span<uint8_t>(buffer),  &flat_container,100);

  if(VERBOSE_TEST){ std::cout << " -------------------- " << std::endl;

    for(auto&it: flat_container.value) {
      std::cout << it.first << " >> " << it.second.convert<double>() << std::endl;
    }
  }

  EXPECT_EQ( flat_container.value[0].first.toStdString() , ("nav_stat/status"));
  EXPECT_EQ( flat_container.value[0].second.convert<int>(), (int)nav_stat.STATUS_GBAS_FIX );
  EXPECT_EQ( flat_container.value[1].first.toStdString() , ("nav_stat/service"));
  EXPECT_EQ( flat_container.value[1].second.convert<int>(), (int)nav_stat.SERVICE_COMPASS );
}

TEST( Deserialize, DeserializeIMU)
//int func()
{
  // We test this because to check if arrays with fixed length work.
  RosIntrospection::Parser parser;

  parser.registerMessageDefinition(
        "imu",
        ROSType(DataType<sensor_msgs::Imu>::value()),
        Definition<sensor_msgs::Imu>::value());

  sensor_msgs::Imu imu;

  imu.header.seq = 2016;
  imu.header.stamp.sec  = 1234;
  imu.header.stamp.nsec = 567*1000*1000;
  imu.header.frame_id = "pippo";

  imu.orientation.x = 11;
  imu.orientation.y = 12;
  imu.orientation.z = 13;
  imu.orientation.w = 14;

  imu.angular_velocity.x = 21;
  imu.angular_velocity.y = 22;
  imu.angular_velocity.z = 23;

  imu.linear_acceleration.x = 31;
  imu.linear_acceleration.y = 32;
  imu.linear_acceleration.z = 33;

  for (int i=0; i<9; i++)
  {
    imu.orientation_covariance[i]         = 40+i;
    imu.angular_velocity_covariance[i]    = 50+i;
    imu.linear_acceleration_covariance[i] = 60+i;
  }

  std::vector<uint8_t> buffer( ros::serialization::serializationLength(imu) );
  ros::serialization::OStream stream(buffer.data(), buffer.size());
  ros::serialization::Serializer<sensor_msgs::Imu>::write(stream, imu);

  FlatMessage flat_container;
  parser.deserializeIntoFlatContainer("imu",  absl::Span<uint8_t>(buffer),  &flat_container,100);


  if(VERBOSE_TEST){

    std::cout << " -------------------- " << std::endl;
    for(auto&it: flat_container.value) {
      std::cout << it.first << " >> " << it.second.convert<double>() << std::endl;
    }
  }

  int index = 0;

  EXPECT_EQ( flat_container.value[index].first.toStdString() , ("imu/header/seq"));
  EXPECT_EQ( flat_container.value[index].second.convert<int>(), 2016 );
  index++;
  EXPECT_EQ( flat_container.value[index].first.toStdString() , ("imu/header/stamp"));
  EXPECT_EQ( flat_container.value[index].second.convert<double>(),   double(1234.567)  );
  EXPECT_EQ( flat_container.value[index].second.convert<ros::Time>(), imu.header.stamp  );
  index++;
  EXPECT_EQ( flat_container.value[index].first.toStdString() , ("imu/orientation/x"));
  EXPECT_EQ( flat_container.value[index].second.convert<int>(), 11 );
  index++;
  EXPECT_EQ( flat_container.value[index].first.toStdString() , ("imu/orientation/y"));
  EXPECT_EQ( flat_container.value[index].second.convert<int>(), 12 );
  index++;
  EXPECT_EQ( flat_container.value[index].first.toStdString() , ("imu/orientation/z"));
  EXPECT_EQ( flat_container.value[index].second.convert<int>(), 13 );
  index++;
  EXPECT_EQ( flat_container.value[index].first.toStdString() , ("imu/orientation/w"));
  EXPECT_EQ( flat_container.value[index].second.convert<int>(), 14 );
  index++;

  for(int i=0; i<9; i++)
  {
    char str[64];
    sprintf(str, "imu/orientation_covariance.%d",i);
    EXPECT_EQ( flat_container.value[index].first.toStdString() , (str) );
    EXPECT_EQ( flat_container.value[index].second.convert<int>(), 40+i );
    index++;
  }

  EXPECT_EQ( flat_container.value[index].first.toStdString() , ("imu/angular_velocity/x"));
  EXPECT_EQ( flat_container.value[index].second.convert<int>(), 21 );
  index++;
  EXPECT_EQ( flat_container.value[index].first.toStdString() , ("imu/angular_velocity/y"));
  EXPECT_EQ( flat_container.value[index].second.convert<int>(), 22 );
  index++;
  EXPECT_EQ( flat_container.value[index].first.toStdString() , ("imu/angular_velocity/z"));
  EXPECT_EQ( flat_container.value[index].second.convert<int>(), 23 );
  index++;

  for(int i=0; i<9; i++)
  {
    char str[64];
    sprintf(str, "imu/angular_velocity_covariance.%d",i);
    EXPECT_EQ( flat_container.value[index].first.toStdString() , (str) );
    EXPECT_EQ( flat_container.value[index].second.convert<int>(), 50+i );
    index++;
  }

  EXPECT_EQ( flat_container.value[index].first.toStdString() , ("imu/linear_acceleration/x"));
  EXPECT_EQ( flat_container.value[index].second.convert<int>(), 31 );
  index++;
  EXPECT_EQ( flat_container.value[index].first.toStdString() , ("imu/linear_acceleration/y"));
  EXPECT_EQ( flat_container.value[index].second.convert<int>(), 32 );
  index++;
  EXPECT_EQ( flat_container.value[index].first.toStdString() , ("imu/linear_acceleration/z"));
  EXPECT_EQ( flat_container.value[index].second.convert<int>(), 33 );
  index++;

  for(int i=0; i<9; i++)
  {
    char str[64];
    sprintf(str, "imu/linear_acceleration_covariance.%d",i);
    EXPECT_EQ( flat_container.value[index].first.toStdString() , (str) );
    EXPECT_EQ( flat_container.value[index].second.convert<int>(), 60+i );
    index++;
  }

  //---------------------------------
//  std::vector< std::pair<SString,std_msgs::Header>> headers;
//  std::vector< std::pair<SString,geometry_msgs::Vector3>> vectors;
//  std::vector< std::pair<SString,geometry_msgs::Quaternion>> quaternions;

//  ExtractSpecificROSMessages(type_map,  main_type,
//                              "imu", buffer,
//                              headers);

//  EXPECT_EQ(headers.size(), 1);
//  const std_msgs::Header& header = headers[0].second;
//  std::string header_prefix =  headers[0].first.toStdString();
//  EXPECT_EQ( header_prefix, "imu/header");
//  EXPECT_EQ(header.seq,        imu.header.seq);
//  EXPECT_EQ(header.stamp.sec,  imu.header.stamp.sec);
//  EXPECT_EQ(header.stamp.nsec, imu.header.stamp.nsec);
//  EXPECT_EQ(header.frame_id,   imu.header.frame_id);

//  ExtractSpecificROSMessages(type_map,  main_type,
//                              "imu", buffer,
//                              quaternions);

//  EXPECT_EQ(quaternions.size(), 1);
//  const geometry_msgs::Quaternion& quaternion = quaternions[0].second;
//  std::string quaternion_prefix =  quaternions[0].first.toStdString();
//  EXPECT_EQ( quaternion_prefix, "imu/orientation");
//  EXPECT_EQ(quaternion.x,  imu.orientation.x);
//  EXPECT_EQ(quaternion.y,  imu.orientation.y);
//  EXPECT_EQ(quaternion.z,  imu.orientation.z);
//  EXPECT_EQ(quaternion.w,  imu.orientation.w);

//  ExtractSpecificROSMessages(type_map,  main_type,
//                              "imu", buffer,
//                              vectors);

//  EXPECT_EQ(vectors.size(), 2);
//  for( const auto& vect_pair: vectors)
//  {
//    if( vect_pair.first.toStdString() == "imu/angular_velocity")
//    {
//      EXPECT_EQ(vect_pair.second.x,  imu.angular_velocity.x);
//      EXPECT_EQ(vect_pair.second.y,  imu.angular_velocity.y);
//      EXPECT_EQ(vect_pair.second.z,  imu.angular_velocity.z);
//    }
//    else if( vect_pair.first.toStdString() == "imu/linear_acceleration")
//    {
//      EXPECT_EQ(vect_pair.second.x,  imu.linear_acceleration.x);
//      EXPECT_EQ(vect_pair.second.y,  imu.linear_acceleration.y);
//      EXPECT_EQ(vect_pair.second.z,  imu.linear_acceleration.z);
//    }
//    else{
//      FAIL();
//    }
//  }
}



TEST( Deserialize, Int16MultiArrayDeserialize)
//int func()
{
  RosIntrospection::Parser parser;

  parser.registerMessageDefinition(
        "multi_array",
        ROSType(DataType<std_msgs::Int16MultiArray>::value()),
        Definition<std_msgs::Int16MultiArray>::value());

  std_msgs::Int16MultiArray multi_array;

  const unsigned N = 6;
  multi_array.layout.data_offset = 42;
  multi_array.data.resize(N);

  for (unsigned i=0; i<N; i++){
    multi_array.data[i] = i;
  }

  std::vector<uint8_t> buffer( ros::serialization::serializationLength(multi_array) );
  ros::serialization::OStream stream(buffer.data(), buffer.size());
  ros::serialization::Serializer<std_msgs::Int16MultiArray>::write(stream, multi_array);

  FlatMessage flat_container;

  EXPECT_NO_THROW(
        parser.deserializeIntoFlatContainer("multi_array",
                                            absl::Span<uint8_t>(buffer),
                                            &flat_container,100)
        );

  if(VERBOSE_TEST){
    std::cout << " -------------------- " << std::endl;

    for(auto&it: flat_container.value) {
      std::cout << it.first << " >> " << it.second.convert<double>() << std::endl;
    }
  }
}

TEST( Deserialize, SensorImage)
//int func()
{
  RosIntrospection::Parser parser;

  parser.registerMessageDefinition( "image_raw",
        ROSType(DataType<sensor_msgs::Image>::value()),
        Definition<sensor_msgs::Image>::value());

  sensor_msgs::Image image;
  image.width = 640;
  image.height = 480;
  image.step = 3*image.width;
  image.data.resize( image.height * image.step );

  std::vector<uint8_t> buffer( ros::serialization::serializationLength(image) );
  ros::serialization::OStream stream(buffer.data(), buffer.size());
  ros::serialization::Serializer<sensor_msgs::Image>::write(stream, image);

  FlatMessage flat_container;

  EXPECT_NO_THROW(
        parser.deserializeIntoFlatContainer("image_raw",
                                            absl::Span<uint8_t>(buffer),
                                            &flat_container,100)
        );

}


// Run all the tests that were declared with TEST()
int main(int argc, char **argv){
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
