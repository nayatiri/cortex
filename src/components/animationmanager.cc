#include "animationmanager.hh"
#include <memory>

Animation_Manager::Animation_Manager(std::shared_ptr<Scene> set_scene) {

  this->m_active_scene = set_scene;
  
}

void Animation_Manager::handle_scene_animations(float m_application_current_time) {

  if(m_active_scene == nullptr)
    return;
  
  // is an animation already running? animate it
  if (m_active_scene->m_camera->m_animation_table &&
      m_active_scene->m_camera->m_animation_table->at(0)->m_trigger_animation) {

    log_error("animation has been marked to triggered -> trying to animate it rn!");

    if (m_active_scene->m_camera->m_animation_table->at(0)->m_start_time <
            m_application_current_time &&
        m_active_scene->m_camera->m_animation_table->at(0)->m_start_time != 0) {

      float start_time =
          m_active_scene->m_camera->m_animation_table->at(0)->m_start_time;
      float num_checkpoints = m_active_scene->m_camera->m_animation_table->at(0)
                                  ->m_checkpoints->size();
      float anim_speed =
          m_active_scene->m_camera->m_animation_table->at(0)->m_animation_speed;
      float delta =
          (m_active_scene->m_camera->m_animation_table->at(0)->m_start_time *
               anim_speed +
           num_checkpoints) -
          m_application_current_time * anim_speed;

      std::cout << start_time << " start_time " << std::endl;
      std::cout << anim_speed << " anim speed " << std::endl;
      std::cout << delta << " delta " << std::endl;
      std::cout << num_checkpoints << " num_checkpoint " << std::endl;

      // animate
      unsigned int tomove_check = std::ceil(num_checkpoints - delta);
      float remainder = tomove_check - (num_checkpoints - delta);
      std::cout << remainder << "remainder" << std::endl;
      if (tomove_check > num_checkpoints - 1) {
        // done animating? reset.
        tomove_check = num_checkpoints - 1;
        log_error("end of anim reached?");
        m_active_scene->m_camera->m_animation_table->at(0)
            ->m_trigger_animation = false;
        m_active_scene->m_camera->m_animation_table->at(0)->m_start_time = 0;
        log_success("animation done!");
      }

      unsigned int tomove_next = tomove_check + 1;
      if (tomove_next > num_checkpoints - 1)
        tomove_next = num_checkpoints - 1;

      glm::vec3 old_campos_anim =
          m_active_scene->m_camera->m_animation_table->at(0)->m_checkpoints->at(
              tomove_check);
      glm::vec3 next_campos_anim =
          m_active_scene->m_camera->m_animation_table->at(0)->m_checkpoints->at(
              tomove_next);
      glm::vec3 old_campos_anim_rot =
          m_active_scene->m_camera->m_animation_table->at(0)
              ->m_checkpoints_rot->at(tomove_check);
      glm::vec3 next_campos_anim_rot =
          m_active_scene->m_camera->m_animation_table->at(0)
              ->m_checkpoints_rot->at(tomove_next);

      glm::vec3 interpolated_camera_pos =
          (remainder * old_campos_anim) + ((1 - remainder) * next_campos_anim);
      glm::vec3 interpolated_camera_rot =
          (remainder * old_campos_anim_rot) +
          ((1 - remainder) * next_campos_anim_rot);

      m_active_scene->m_camera->m_cameraPos = interpolated_camera_pos;
      m_active_scene->m_camera->m_cameraLookAt = interpolated_camera_rot;

      std::cout << tomove_check << " tomove_check " << std::endl;

      log_success("anim step done!");

    } else {
      log_error("no animation running");
      std::cout << "times are - st - ct : " << m_active_scene->m_camera->m_animation_table->at(0)->m_start_time << " " << m_application_current_time << std::endl;
      }
  }
}
