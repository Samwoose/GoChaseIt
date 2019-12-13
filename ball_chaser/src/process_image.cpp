#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ROS_INFO_STREAM("drive the robot");
    
    // Request a service to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x; //should have same name as what I define in srv file
    srv.request.angular_z = ang_z;

    // Call the command_robot service and pass the requested direction
    if(!client.call(srv)){
        ROS_ERROR("Failed to call service command_robot") ;
    }
    
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    bool presence_flag = false;
    int location_step_rows = -1 ;
    int location_height = -1; //initialize it with -1. It indicates white_pixel is not in the image. 
    int location_step = -1; //initialize it with -1
    int three = 3; // it indicates 3 division of whole step.
    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    
    int num_of_points_in_pixel = 3 ;
    int first_loc_in_pixel = 0;
    int second_loc_in_pixel = 1;
    int third_loc_in_pixel = 2;
    
    // Loop through each pixel in the image and check if its equal to the white pixel
    for(int i = 0; i < (img.height * img.step)/num_of_points_in_pixel ; i++){
        if((img.data[first_loc_in_pixel + + num_of_points_in_pixel*i] + img.data[second_loc_in_pixel + num_of_points_in_pixel*i] + img.data[third_loc_in_pixel + + num_of_points_in_pixel*i]) - (white_pixel*num_of_points_in_pixel) == 0 ){
            presence_flag = true;
            location_step_rows = i ;
            break;
        }
    }

    //If the image has white pixel check where it falls into and call drive_robot() to make a move for robot
    if(presence_flag == true){
        int location_step = (location_step_rows % (img.step/three))*three ; //get location of step
        int starting = 0 ;
        int left_max = (img.step/three/num_of_points_in_pixel)*three ; //(whole length/3/3)*3
        int mid_max = 2*(img.step/three/num_of_points_in_pixel)*three ; //double (whole length/3/3)*3
        int right_max = img.step ;
    
        if((location_step >= starting) && (location_step <= left_max)){
            //left
            drive_robot(0.0, 0.5);
        }
        else if((location_step > left_max) && (location_step <= mid_max)){
            //forward
            drive_robot(0.2, 0.0);
        }
        else if((location_step >= mid_max)&&(location_step <= right_max)){
            //right
            drive_robot(0.0, -0.5);
        }
          
    }


    


    // If the image doesn't have white pixel stop 
    if(presence_flag == false){
        //stop
        drive_robot(0.0, 0.0);
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
