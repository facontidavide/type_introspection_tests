<<PackageHeader(ros_type_introspection)>> <<TOC(4)>>

== Overview ==

This simple library extracts information from a ROS Message, even when its type is unknown at compilation time.

Have you ever wanted to build an app that can subscribe to any topic and extract its content, or can read data from any rosbag? What if the topic and/or the bag contains user defined ROS types ignored at compilation time?

The common solution in the ROS ecosystem is to use Python, that provides the needed introspection. Tools, for instance, like rqt_plot and rqt_bag took this approach. This library implements a C++ alternative.

= Some background =
The ROS Message Types can be described as a [[https://en.wikipedia.org/wiki/Interface_description_language|Interface Description Language]]. This approach is very well known and commonly used on the web and in distributed systems in general.

A [[http://wiki.ros.org/rosmsg|rosmsg]] is defined by the user; an "IDL compiler", i.e. [[http://wiki.ros.org/gencpp|gencpp]], reads this schema and generates a header file that contains the source code that the user shall include in his/her applications. The inclusion of this header file is needed on both the publisher ''and'' the subscriber sides.

This approach provides strong and type-safe contracts between the producer and the consumer of the message and, additionally, is needed to implements a fast serialization / deserialization mechanism.

The only "problem" is that in very few use cases (for instance if you want to build a plugin to [[https://github.com/bcharrow/matlab_rosbag|load ROS bags in MATLAB]]) you don't know in advance which ROS Messages you will need to read. Therefore, you won't be able to include the necessary header files.

== Ros Type Introspection ==

The library is composed of three main modules:

 * '''Parser''': it performs introspection of a Ros Message using the schema stored in `ros::message_traits::Definition`.

 * '''Deserializer''': using the schema built by the parsed, it can extract the actual values from a raw message.

 * '''Renamer''': last but not least, the library offers as well an easy way to remap/rename the data using a simple set of rules. This can be very handy in multiple scenarios that are very common in ROS.

This library is particularly useful to extract data from two type-erasing classes provided by ROS itself:

1. [[http://docs.ros.org/indigo/api/topic_tools/html/classtopic__tools_1_1ShapeShifter.html|topic_tools::ShapeShifter]]: a type used to subscribe to any topic, regardless of the original type.

2. [[http://docs.ros.org/indigo/api/rosbag_storage/html/c++/classrosbag_1_1MessageInstance.html|rosbag::MessageInstance]]: the generic type commonly used to read data from a ROS bag.

== Usage ==

=== The Parser ===

To introspect a ROS message we need its definition and datatype.

A single function called '''!RosIntrospection::buildROSTypeMapFromDefinition''' extracts a list of types which will be stored in '''!RosIntrospection::ROSTypeList'''.

Suppose for instance that ROSTypeList contains the definition of   [[http://docs.ros.org/kinetic/api/geometry_msgs/html/msg/Pose.html|geometry_msgs::Pose]]: we can print the list of fields and their type as follows:

{{{#!cplusplus
void printRosTypeList(const RosIntrospection::ROSTypeList& type_list)
{
    for (const ROSMessage& msg: type_list) {
        std::cout << "\n" << msg.type().baseName() <<" : " << std::endl;
        for (const ROSField& field : msg.fields() ){
            std::cout  << "\t" << field.name()
                       <<" : " << field.type().baseName() << std::endl;
        }
    }
}
}}}
The output will be

{{{
   geometry_msgs/Pose :
      position : geometry_msgs/Point
      orientation : geometry_msgs/Quaternion

   geometry_msgs/Point :
      x : float64
      y : float64
      z : float64

   geometry_msgs/Quaternion :
      x : float64
      y : float64
      z : float64
      w : float64
}}}
In other words, not only [[http://docs.ros.org/kinetic/api/geometry_msgs/html/msg/Pose.html|geometry_msgs::Pose]] but also [[http://docs.ros.org/kinetic/api/geometry_msgs/html/msg/Point.html|geometry_msgs::Point]] and [[http://docs.ros.org/kinetic/api/geometry_msgs/html/msg/Quaternion.html|geometry_msgs::Quaternion]] is extracted by '''!RosIntrospection::buildROSTypeMapFromDefinition'''.

=== The Deserializer ===

Once the schema is available in the form of a '''ROSTypeList''' we are able to extract valuable information from a "raw" message.

We don't have the support of the C++ typesystem, which was provided by the included file generated by the [[gencpp]], therefore the fields of the message can __not__ be "composed" into a ''struct'' or ''class''.

The only data structure that can contain our data is a '''flat''' vector that stores simple key-value pairs. We avoid using std::map to make the container more cache-friendly.

{{{#!cplusplus
  typedef struct{
    StringTree tree;
    std::vector< std::pair<StringTreeLeaf, double> >  value;
    std::vector< std::pair<StringTreeLeaf, SString> > name;
    std::vector< std::pair<SString, double> > renamed_value;
  }ROSTypeFlat;
}}}
'''!StringTree''' is a data structure inspired by [[https://en.wikipedia.org/wiki/Trie|Trie]]. The only difference is that each node contains an entire substring, not a single character. This is also known as "suffix string tree".

'''!StringTreeLeaf''' is a terminal node of the tree; most of the user just need to know that the name of the key can be built using '''!StringTreeLeaf::toStr()'''.

The data structure itself reveals some of the main limitations of the parser:

 * It is not well suited for large arrays (hundred or thousand of elements), such as images, maps, point clouds, etc. Very large arrays are simply discarded. This may change in the future.

 * A double is used as a "conservative" type to store any builtin integral type. This makes the code simpler and avoid the need of type erasing techniques, that would be less efficient anyway.

 * '''SString''' is used. It is an alternative implementation of std::string with a better ''Small Object Optimization''. It is considerably faster than std::string in many cases, because it use stack allocation instead of heap allocation.

Let's consider for example an instance of [[http://docs.ros.org/kinetic/api/sensor_msgs/html/msg/JointState.html|sensor_msgs::JointState]] built as follows:

{{{#!cplusplus
sensor_msgs::JointState joint_state;

joint_state.header.seq = 2016;
joint_state.header.stamp.sec  = 1234;
joint_state.header.stamp.nsec = 567*1000*1000;
joint_state.header.frame_id = "base_frame";

joint_state.name.resize( 2 );
joint_state.position.resize( 2 );
joint_state.velocity.resize( 2 );
joint_state.effort.resize( 2 );

joint_state.name[0] = "first_joint";
joint_state.position[0]= 10;
joint_state.velocity[0]= 11;
joint_state.effort[0]= 12;

joint_state.name[1] = "second_joint";
joint_state.position[1]= 20;
joint_state.velocity[1]= 21;
joint_state.effort[1]= 22;
}}}
If we print the Key/Value pairs of ROSTypeFlat::value and ROSTypeFlat::name we get this output:

{{{
/// Elements of ROSTypeFlat::value
JointState.header.seq >> 2016.0
JointState.header.stamp >> 1234.57
JointState.position.0 >> 10.0
JointState.position.1 >> 20.0
JointState.velocity.0 >> 11.0
JointState.velocity.1 >> 21.0
JointState.effort.0 >> 12.0
JointState.effort.1 >> 23.0

/// Elements of ROSTypeFlat::name
JointState.header.frame_id >> base_frame
JointState.name.0 >> first_joint
JointState.name.1 >> second_joint
}}}

=== The Renamer ===

At first this functionality doesn't look like much, but you will quickly realize that it is extremely convenient in a number of common use cases. For the records, the decision to use StringTree is mostly focused on making the renamer considerably faster. Let's consider again the frequently used type [[http://docs.ros.org/kinetic/api/sensor_msgs/html/msg/JointState.html|sensor_msgs::JointState]].

It uses the field "name" to assign an identifier to a specific index in the other arrays (position, velocity, effort). This means of course that you have to serialize, deserialize and compare these strings in every single message.

As we saw in the previous section, the Deserialized is order-dependant, while the Key-Value approach in ROS is order-independent.  What would happen if the order of the identifiers is shuffled in different message? What if another message contains only the a single !JointState with

 . !JointState.name.0 = second_joint ?

If you have ever tried to visualize a [[tf]] or [[tf2]] message in [[rqt_plot]], you know what I am talking about...

What we ideally want in the !JointState example is:

{{{
JointState.header.seq >> 2016.0
JointState.header.stamp >> 1234.57
JointState.header.frame_id >> base_frame

JointState.first_joint.pos >> 10
JointState.first_joint.vel >> 11
JointState.first_joint.eff >> 12

JointState.second_joint.pos >> 20
JointState.second_joint.vel >> 21
JointState.second_joint.eff >> 22
}}}
This can be achieved using the function:

{{{#!cplusplus
  void applyNameTransform(const std::vector<SubstitutionRule> &rules,
                          ROSTypeFlat* container);
}}}
This function will use a set of rules to fill the vector ROSTypeFlat::renamed_value using the data contained in

 . ROSTypeFlat::value

and

 . ROSTypeFlat::name.

The rules used to perform the remapping in the JointState example are:

{{{#!cplusplus
std::vector<SubstitutionRule> rules;
rules.push_back( SubstitutionRule( "position.#", "name.#", "@.pos" ));
rules.push_back( SubstitutionRule( "velocity.#", "name.#", "@.vel" ));
rules.push_back( SubstitutionRule( "effort.#", "name.#", "@.eff" ));
}}}
These rules are pretty easy to use once they are understood. For instance, let's consider the following example:

 . '''the rule'''   !SubstitutionRule( "position.#", "name.#", "@.pos" ) '''is using'''   !JointState.name.0 = first_joint '''to convert''' !JointState.position.0 = 10 '''into'''       !JointState.first_joint.pos = 10

The first argument, "'''position.#'''", means: "find any element in ROSTypeFlat::value which contains the pattern [position.#] where '''#''' is a number".

 . !JointState.position.0 = 10

The second argument, "'''name.#'''", means: "find the element in ROSTypeFlat::name which contains the pattern [name.#] where '''#''' is the same number found in the previous pattern".

 . !JointState.name.0 = first_joint

The third argument, "'''@.pos'''", means: "substitute the pattern found in 1. with this string, where the symbol '''@''' represents the name found in 2". The final result is therefore:

 . !JointState.first_joint.pos = 10

## AUTOGENERATED DON'T DELETE
## CategoryPackage
