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

vec2 closest_point_on_line_segment(vec2 p, vec2 a, vec2 b) {
    vec2 ab = b - a;
    vec2 ap = p - a;
    float t = dot(ap, ab) / dot(ab, ab);
    t = clamp(t, 0.0, 1.0);
    return a + t * ab;
}

vec2 project_to_screen_space(vec3 world_pos, float aspect_ratio) {
    vec4 clip = projection * view * vec4(world_pos, 1.0);
    if (clip.w <= 0.0) return vec2(100000); // invalid, far away
    vec2 ndc = (clip.xy / clip.w);
    ndc.x *= aspect_ratio;
    return ndc;
}

float distance_from_edge_segment(vec3 edge_start_world, vec3 edge_end_world, float aspect_ratio, vec2 frag_pos_ndc) {
    vec2 start_ndc = project_to_screen_space(edge_start_world, aspect_ratio);
    vec2 end_ndc   = project_to_screen_space(edge_end_world, aspect_ratio);

    if (abs(start_ndc.x) > 1000.0 || abs(end_ndc.x) > 1000.0) {
        return 100000.0;
    }

    vec2 closest_ndc = closest_point_on_line_segment(frag_pos_ndc, start_ndc, end_ndc);
    return length(frag_pos_ndc - closest_ndc);
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

    int edges[12][2] = {
        {0, 1}, {1, 3}, {3, 2}, {2, 0}, // bottom face
        {4, 5}, {5, 7}, {7, 6}, {6, 4}, // top face
        {0, 4}, {1, 5}, {2, 6}, {3, 7}  // verticals
    };

    float aspect_ratio = screen_width / screen_height;

    vec2 fragPos = gl_FragCoord.xy / vec2(screen_width, screen_height);
    fragPos = fragPos * 2.0 - 1.0;
    fragPos.x *= aspect_ratio;

    float lowest_distance = 100000.0;
    vec3 closest_point_world = vec3(0.0);
    int paint_flag = 0;

    for (int i = 0; i < 12; i++) {
        vec3 start = corners[edges[i][0]];
        vec3 end   = corners[edges[i][1]];

        vec3 mid = (start + end) * 0.5;
        float dist_to_cam = length(mid - camera_position);
        float inv_dist = 1.0 / max(dist_to_cam, 0.001);
        float scaled_radius = inv_dist * radius * 0.1;

        float dist_to_edge = distance_from_edge_segment(start, end, aspect_ratio, fragPos);

        if (dist_to_edge < scaled_radius && dist_to_edge < lowest_distance) {
            lowest_distance = dist_to_edge;
            closest_point_world = mid; // or you could interpolate based on closest point
            paint_flag = 1;
        }
    }

    if (paint_flag == 1) {
        set_fragment_depth(closest_point_world);
        float alpha = 1.0 - smoothstep(radius * 0.1 - 0.005, radius * 0.1 + 0.005, lowest_distance);
        if (alpha < 0.01) discard;
        FragColor = vec4(1.0, 0.0, 0.0, alpha);
    } else {
        discard;
    }
}