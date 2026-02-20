#include "./panini.hh"

glm::mat4 panini::get_panini_projection_matrix(float fovy,
					       float aspect,
					       float zNear,
					       float zFar,
					       float d) {
  
  const float eps = 1e-6f;

    float f = 1.0f / tanf(0.5f * fovy);

    d = glm::max(d, eps);

    float s = (d + 1.0f) / (d + aspect);

    glm::mat4 P(0.0f);

    P[0][0] = f * s / aspect;
    P[1][1] = f;

    P[2][2] = (zFar + zNear) / (zNear - zFar);
    P[2][3] = -1.0f;

    P[3][2] = (2.0f * zFar * zNear) / (zNear - zFar);

    return P;
};
