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

     //not multiplying with model matrix, because point_location_world is already in world space instead of model space
     vec4 point_screen_space = projection * view * vec4(point_location_world, 1.0);
     if(point_screen_space.w <= 0.0) { discard; }
     
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

     vec3 corners[8];
     int idx = 0;
     for (int x = 0; x < 2; x++)
     for (int y = 0; y < 2; y++)
     for (int z = 0; z < 2; z++) {
         corners[idx++] = mix(box_position_min, box_position_max, vec3(x, y, z));
     }

     float aspect_ratio = screen_width / screen_height;
     
     // get fragPos in ndc
     vec2 fragPos = gl_FragCoord.xy / vec2(screen_width, screen_height);
     fragPos = fragPos * 2.0 - 1.0;
     fragPos.x *= aspect_ratio;

     float lowest_distance_buffer = 100000;
     vec3 closest_corner = vec3(0.0,0.0,1.0);
     int paint_flag = 0;

     for(int i = 0; i < corners.length(); i++) {

     //do this for all corners in array
     float distance = distance_from_point(corners[i], aspect_ratio, fragPos); 
     float distance_point_cam = length(corners[i] - camera_position);
     
     float inv_dist = 1.0 / max(distance_point_cam, 0.001);
     float radius_scaled_by_position = inv_dist * radius * 0.1;

         if(distance < radius_scaled_by_position && distance < lowest_distance_buffer) {
             lowest_distance_buffer = distance;
	     closest_corner = corners[i];
	     paint_flag = 1;
         }

     }

     // paint this abomination
     if(paint_flag == 1) {
         set_fragment_depth(closest_corner);
	 float alpha = 1.0 - smoothstep(radius - 0.005, radius + 0.005, lowest_distance_buffer);
	 if (alpha < 0.01) discard;
	 FragColor = vec4(1.0, 0.0, 0.0, alpha);
	 //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
     } else {discard;}
       
}