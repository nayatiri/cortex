#version 450 core

out vec4 FragColor;

uniform float radius;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 box_position_min;
uniform vec3 box_position_max;
uniform vec3 camera_position;

uniform float screen_width;
uniform float screen_height;	

float distance_from_point(vec3 point_location_world, float aspect_ratio, vec2 frag_pos_ndc) {

     vec4 point_screen_space = projection * view * model * vec4(point_location_world, 1.0);

     // clip space to ndc -> then get 2d coord
     vec2 point_screen_position = (point_screen_space.xyz / point_screen_space.w ).xy;
     point_screen_position.x *= aspect_ratio;

     float distance = length(frag_pos_ndc - point_screen_position);
          
     return distance;

}

void set_fragment_depth(vec3 point_position) {

     vec4 point_clip = projection * view * model * vec4(point_position, 1.0);
     
     float ndc_z = point_clip.z / point_clip.w;  
     float window_z = ndc_z * 0.5 + 0.5;
     gl_FragDepth = window_z;

}

void main() {

     vec3 corners[2];

     corners[0] = box_position_min;
     corners[1] = box_position_max;

     float aspect_ratio = screen_width / screen_height;
     
     // get fragPos in ndc
     vec2 fragPos = gl_FragCoord.xy / vec2(screen_width, screen_height);
     fragPos = fragPos * 2.0 - 1.0;
     fragPos.x *= aspect_ratio;

     //do this for all corners in array
     float distance = distance_from_point(corners[0], aspect_ratio, fragPos); 
     float distance_point_cam = length(corners[0] - camera_position);
     float radius_scaled_by_position = (1/distance_point_cam) * radius * 0.1;

//todo make it run calculation for all corners. then make a distance buffer that logs the closest distance, if closest distance from all corners < adjusted radius, then pain frag

     if(distance < radius_scaled_by_position) {
     
         set_fragment_depth(corners[0]);
         FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	 
     } else {
       	 discard;
     }
     
}