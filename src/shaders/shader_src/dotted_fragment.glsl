#version 450 core

out vec4 FragColor;

uniform float radius;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 line_color;

uniform vec3 line_position_min;
uniform vec3 line_position_max;
uniform vec3 camera_position;

uniform float screen_width;
uniform float screen_height;

uniform float dot_spacing_px;
uniform float dot_radius_px;

vec2 project_to_screen_space(vec3 world_pos) {
    vec4 clip = projection * view * vec4(world_pos, 1.0);
    if (clip.w <= 0.0) return vec2(100000); // invalid, behind camera
    return clip.xy / clip.w; // true NDC in [-1,1]
}

vec2 closest_point_on_segment(vec2 p, vec2 a, vec2 b) {
    vec2 ab = b - a;
    vec2 ap = p - a;
    float t = dot(ap, ab) / max(dot(ab, ab), 1e-8);
    t = clamp(t, 0.0, 1.0);
    return a + t * ab;
}

void set_fragment_depth(vec3 world_pos) {
    vec4 clip = projection * view * vec4(world_pos, 1.0);
    float ndc_z = clip.z / clip.w;
    float window_z = ndc_z * 0.5 + 0.5; // [0,1]
    gl_FragDepth = window_z;
}

void main() {

    vec2 frag_pos_ndc = (gl_FragCoord.xy / vec2(screen_width, screen_height)) * 2.0 - 1.0;

    vec2 start_ndc = project_to_screen_space(line_position_min);
    vec2 end_ndc   = project_to_screen_space(line_position_max);

    if (any(lessThan(start_ndc, vec2(-1e6))) || any(greaterThan(start_ndc, vec2(1e6))) ||
        any(lessThan(end_ndc, vec2(-1e6))) || any(greaterThan(end_ndc, vec2(1e6)))) {
        discard;
    }

    vec2 closest_on_line_ndc = closest_point_on_segment(frag_pos_ndc, start_ndc, end_ndc);
    float dist_to_line_ndc = length(frag_pos_ndc - closest_on_line_ndc);

    float pixel_size_ndc_x = 2.0 / screen_width;
    float pixel_size_ndc_y = 2.0 / screen_height;
    float avg_pixel_size_ndc = (pixel_size_ndc_x + pixel_size_ndc_y) * 0.5;
    
    vec3 midpoint = vec3(line_position_min + line_position_max) * 0.5;
    float distance_point_cam = length(midpoint - camera_position);
    float radius_adjusted = (1/distance_point_cam) * radius * avg_pixel_size_ndc;

//    if (dist_to_line_ndc > radius_adjusted) {
//        discard;
//    }

    float t = dot(frag_pos_ndc - start_ndc, end_ndc - start_ndc) /
              max(dot(end_ndc - start_ndc, end_ndc - start_ndc), 1e-8);
    t = clamp(t, 0.0, 1.0);
    vec3 closest_world_point = mix(line_position_min, line_position_max, t);

    set_fragment_depth(closest_world_point);
    
    float line_length_ndc = length(end_ndc - start_ndc);
    float line_pos_ndc = t * line_length_ndc;

    float spacing_ndc = dot_spacing_px * avg_pixel_size_ndc;
    float dot_radius_ndc = dot_radius_px * avg_pixel_size_ndc;

    float local = mod(line_pos_ndc, spacing_ndc) - spacing_ndc * 0.5;
    float dot_sdf = abs(local) - dot_radius_ndc;

    float line_sdf = dist_to_line_ndc - radius_adjusted;

    float sdf = max(line_sdf, dot_sdf);

    float aa = avg_pixel_size_ndc * 1.5;
    float alpha = 1.0 - smoothstep(0.0, aa, sdf);

    if (alpha < 0.01) discard;
    FragColor = vec4(line_color, alpha);
}
