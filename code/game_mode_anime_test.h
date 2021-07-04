/* date = April 24th 2021 4:53 pm */

#ifndef GAME_MODE_ANIME_TEST_H
#define GAME_MODE_ANIME_TEST_H

struct Anime_Component {
    Anime_ID anime_id;
    u32 current_frame_index;
    f32 timer_per_frame;
    f32 duration_per_frame;
};

static inline void
create(Anime_Component* a,
       Anime_ID id, 
       f32 duration_per_frame) 
{
    a->anime_id = id;
    a->current_frame_index = 0;
    a->timer_per_frame = 0.f;
    a->duration_per_frame = duration_per_frame;
}

static inline void
update(Anime_Component* a, f32 dt) {
    Anime* anime = g_assets->get_anime(ANIME_KARU_FRONT);
    a->timer_per_frame += dt;
    
    a->timer_per_frame += dt;
    if (a->timer_per_frame >= 0.25f) 
    {
        a->timer_per_frame = 0.f;
        ++a->current_frame_index;
        if (a->current_frame_index >= anime->frame_count) {
            a->current_frame_index = 0;
        }
    }
}

struct Game_Mode_Anime_Test_Entity {
    v3f pos;
    v3f size;
    
    Anime_Component anime_com;
};

struct Game_Mode_Anime_Test {
    Game_Mode_Anime_Test_Entity entity;
    v2f prev_mouse_pos;
};

static inline void 
init_anime_test_mode(Permanent_State* perm_state) {
    Game_Mode_Anime_Test* mode = perm_state->anime_test_mode;     
    mode->entity.pos = v3f::create(0.f, 0.f, 0.f);
    mode->entity.size = v3f::create(48.f*2, 48.f*2, 1.f);
    create(&mode->entity.anime_com, ANIME_KARU_FRONT, 0.25f);
    
}




static inline void
update_anime_test_mode(Permanent_State* perm_state, 
                       Transient_State* tran_state,
                       f32 delta_time) 
{
    Game_Mode_Anime_Test* mode = perm_state->anime_test_mode;     
    Game_Mode_Anime_Test_Entity* entity = &mode->entity;
    
    update(&entity->anime_com, delta_time);
    
    // TODO(Momo): Draw function?
    m44f t = m44f::create_translation(entity->pos.x, entity->pos.y, entity->pos.z);
    m44f s = m44f::create_scale(entity->size.x, entity->size.y, entity->size.z);
    
    c4f color = C4F_WHITE;
    Anime_Component* anime_com = &entity->anime_com;
    {
        Anime* anime = g_assets->get_anime(anime_com->anime_id);
        Image_ID image_id = anime->frames[anime_com->current_frame_index];
        draw_textured_quad_from_image(image_id,
                                      t*s,
                                      color);
        
    }
    
}


#endif //GAME_MODE_ANIME_TEST_H
