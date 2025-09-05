#version 450 core

out vec4 FragColor;

uniform float radius;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 point_position;
uniform vec3 camera_position;

uniform float screen_width;
uniform float screen_height;	

void main() {     

     vec4 point_world = model * vec4(point_position, 1.0);

     vec4 point_clip = projection * view * point_world;
     if(point_clip.w <= 0.0) { discard; }

     float wth_ratio = screen_width / screen_height;

     vec3 point_ndc = point_clip.xyz / point_clip.w;

     vec2 point_screen_position = point_ndc.xy;

     vec2 fragPos = gl_FragCoord.xy / vec2(screen_width, screen_height);
     fragPos = fragPos * 2.0 - 1.0;

     float aspect_ratio = screen_width / screen_height;
     fragPos.x *= aspect_ratio;
     point_screen_position.x *= aspect_ratio;

     float distance = length(fragPos - point_screen_position);

     if(abs(point_screen_position.x) > 1.0 || abs(point_screen_position.y) > 1.0) { discard; }

     float distance_point_cam = length(point_position - camera_position);

     float radius_adjusted = (1/distance_point_cam) * radius;

     if(distance < radius_adjusted) {
     	 FragColor = vec4(1.0, 0.0, 0.0, 1.0);	
     } else {
	 discard;
      }
}