#version 450 core

out vec4 FragColor;

uniform float radius;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 point_color;

uniform vec3 point_position;
uniform vec3 camera_position;

uniform float screen_width;
uniform float screen_height;	

void main() {

     // project +clip
     vec4 point_clip = projection * view * vec4(point_position, 1.0);
     if(point_clip.w <= 0.0) { discard; }

     float aspect_ratio = screen_width / screen_height;

     // move from clip into ndc
     vec2 point_screen_position = ( point_clip.xyz / point_clip.w ).xy;

     vec2 fragPos = gl_FragCoord.xy / vec2(screen_width, screen_height);
     fragPos = fragPos * 2.0 - 1.0;

     fragPos.x *= aspect_ratio;
     point_screen_position.x *= aspect_ratio;

     float distance = length(fragPos - point_screen_position);
     float distance_point_cam = length(point_position - camera_position);
     float radius_adjusted = (1/distance_point_cam) * radius * 0.1; // 0.1 magic number hellyear

     //adjust depth buffer, this shit lowkey magic nojoke
     float ndc_z = point_clip.z / point_clip.w;  
     float window_z = ndc_z * 0.5 + 0.5;
     gl_FragDepth = window_z;

     //write color
     if(distance < radius_adjusted) {
     	 FragColor = vec4(point_color, 1.0);	
     } else {
	 discard;
     }
      
}