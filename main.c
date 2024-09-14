/* Copyright (C) 2016 Marcelo Serrano Zanetti - All Rights Reserved
 *
 * Licensed under the GNU GPL V3.0 license. All conditions apply.
 *
 * Powered by Allegro: http://liballeg.org/license.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_primitives.h>

const float FPS = 60;
const int SCREEN_W = 640;
const int SCREEN_H = 480;
const int pombo_TAMANHO = 20;
float pombo_velocidade = 2.0;

enum GameState {
    MENU,
    PLAYING,
    VICTORY,
    DEFEAT
};

int main(int argc, char **argv)
{
    srand(time(NULL));
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_AUDIO_STREAM *audio_theme = NULL;
    ALLEGRO_BITMAP *cursor = NULL;
    ALLEGRO_FONT *font = NULL;

    if (!al_init())
    {
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }

    timer = al_create_timer(1.0 / FPS);
    if (!timer)
    {
        fprintf(stderr, "failed to create timer!\n");
        return -1;
    }

    bool redraw = true;
    al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_WINDOWED);
    display = al_create_display(SCREEN_W, SCREEN_H);

    if (!display)
    {
        fprintf(stderr, "failed to create display!\n");
        al_destroy_timer(timer);
        return -1;
    }

    if (!al_install_keyboard())
    {
        fprintf(stderr, "failed to initialize the keyboard!\n");
        return -1;
    }

    if (!al_install_mouse())
    {
        fprintf(stderr, "failed to initialize the mouse!\n");
        return -1;
    }

    if (!al_init_image_addon())
    {
        fprintf(stderr, "failed to initialize the image addon!\n");
        return -1;
    }

    ALLEGRO_BITMAP *pombo = NULL;
    ALLEGRO_BITMAP *tela_inicial = NULL;
    ALLEGRO_BITMAP *derrota = NULL;
    ALLEGRO_BITMAP *vitoria = NULL;
    ALLEGRO_BITMAP *background = NULL;
    ALLEGRO_FONT *fontes_8bit_wonder_50 = NULL;

    int contador = 5400;
    int relogio = 90;
    bool contador_ativo = false;
    bool relogio_ativo = false;
    float pombo_pos_x = SCREEN_W / 2.0 - pombo_TAMANHO / 2.0;
    float pombo_pos_y = SCREEN_H / 2.0 - pombo_TAMANHO / 2.0;
    float pombo_vx = 4.0;
    float pombo_vy = 4.0;
    int click = 0;
    int savebvx;
    int savebvy;

    pombo = al_load_bitmap("imagens/pombo.png");
    background = al_load_bitmap("imagens/fundo.png");
    vitoria = al_load_bitmap("imagens/vitoria.png");
    derrota = al_load_bitmap("imagens/derrota.png");
    tela_inicial = al_load_bitmap("imagens/tela_inicial.png");
    cursor = al_load_bitmap("imagens/cursor.png");

    if (!pombo)
    {
        fprintf(stderr, "failed to create the pombo bitmap!\n");
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    if (!al_install_audio())
    {
        fprintf(stderr, "failed to initialize audio\n");
        return -1;
    }

    if (!al_init_acodec_addon())
    {
        fprintf(stderr, "failed to initialize audio codec!\n");
        return -1;
    }

    if (!al_reserve_samples(10))
    {
        fprintf(stderr, "failed to allocate audio channels.\n");
        return -1;
    }

    audio_theme = al_load_audio_stream("musica/tema.ogg", 4, 1024);
    if (!audio_theme)
    {
        fprintf(stderr, "failed to initialize theme music.\n");
        return -1;
    }

    if (!background)
    {
        fprintf(stderr, "failed to create the background bitmap!\n");
        al_destroy_display(display);
        al_destroy_timer(timer);
        al_destroy_bitmap(pombo);
        return -1;
    }

    ALLEGRO_MOUSE_CURSOR *custom_cursor = al_create_mouse_cursor(cursor, 0, 0);
    al_set_mouse_cursor(display, custom_cursor);

    al_set_target_bitmap(al_get_backbuffer(display));
    event_queue = al_create_event_queue();
    if (!event_queue)
    {
        fprintf(stderr, "failed to create event_queue!\n");
        al_destroy_bitmap(pombo);
        al_destroy_bitmap(background);
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_mouse_event_source());

    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();
    al_start_timer(timer);

    int gameState = MENU;

    font = al_create_builtin_font();
    if (!font)
    {
        fprintf(stderr, "failed to create font!\n");
        al_destroy_bitmap(pombo);
        al_destroy_bitmap(background);
        al_destroy_display(display);
        al_destroy_timer(timer);
        al_destroy_event_queue(event_queue);
        al_destroy_mouse_cursor(custom_cursor);
        return -1;
    }

    while (1)
    {
        ALLEGRO_EVENT ev;

        bool menu = true;
        bool sair = false;
        bool playing = false;
        bool vitoria_jogo = false;
        bool derrota_jogo = false;

        al_wait_for_event(event_queue, &ev);

        switch (gameState)
        {
        case MENU:
            al_rewind_audio_stream(audio_theme);
            al_attach_audio_stream_to_mixer(audio_theme, al_get_default_mixer());
            al_set_audio_stream_playmode(audio_theme, ALLEGRO_PLAYMODE_LOOP);
            al_set_audio_stream_playing(audio_theme, true);
            if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_SPACE)
            {
                contador_ativo = true;
                relogio_ativo = true;
                gameState = PLAYING;
                al_set_audio_stream_playing(audio_theme, true);
            }
            else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                al_set_audio_stream_playing(audio_theme, true);
            }
            break;

        case PLAYING:
            if (ev.type == ALLEGRO_EVENT_TIMER)
            {
                al_set_audio_stream_playing(audio_theme, true);
                if (contador_ativo && relogio_ativo)
                {
                    contador--;
                    if (contador % 60 == 0)
                    {
                        relogio--;
                    }
                }
                redraw = true;
            }

            // Lógica do jogo
            // Obter a posição do mouse
            int mouse_x = ev.mouse.x;
            int mouse_y = ev.mouse.y;

            // Calcular a direção do pombo em relação à posição do mouse
            float dx = pombo_pos_x - mouse_x;
            float dy = pombo_pos_y - mouse_y;
            float dist = sqrt(dx * dx + dy * dy);
            float dir_x = dx / (dist * 1.8);
            float dir_y = dy / (dist * 1.8);

            // Definir uma velocidade máxima para o pombo
            float pombo_speed;
            pombo_speed = 3.0;

            // Atualizar a posição do pombo com base na direção e velocidade
            pombo_pos_x = pombo_pos_x + dir_x * pombo_speed;
            pombo_pos_y = pombo_pos_y + dir_y * pombo_speed;

            // Verificar se o pombo está dentro dos limites da tela
            if (pombo_pos_x < 0)
            {
                pombo_pos_x = 0;
            }

            else if(pombo_pos_x + pombo_TAMANHO > SCREEN_W)
            {
                pombo_pos_x = SCREEN_W - pombo_TAMANHO;
            }

            if (pombo_pos_y < 0)
            {
                pombo_pos_y = 0;
            }

            else if(pombo_pos_y + pombo_TAMANHO > SCREEN_H)
            {
                pombo_pos_y = SCREEN_H - pombo_TAMANHO;
            }

            // Verificar se o pombo está perto do limite da parede
            if (pombo_pos_x <= 8 || pombo_pos_x >= SCREEN_W - pombo_TAMANHO - 8 ||
            pombo_pos_y <= 8 || pombo_pos_y >= SCREEN_H - pombo_TAMANHO - 8)
            {
                // Teletransportar o pombo para uma posição aleatória dentro da tela
                pombo_pos_x = rand() % (SCREEN_W - pombo_TAMANHO);
                pombo_pos_y = rand() % (SCREEN_H - pombo_TAMANHO);
            }

            if (contador == 0)
            {
                gameState = DEFEAT;
                contador = 5400;
                relogio = 90;
            }

            // Diminuir a velocidade do pombo gradualmente ao longo do tempo
            if (pombo_vx < 1)
            {
                pombo_vx = 1;
            }

            if (pombo_vy < 1)
            {
                pombo_vy = 1;
            }

            if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
            {
                // Verificar se o pombo foi clicado
                if (ev.mouse.button == 1 && ev.mouse.x >= pombo_pos_x && ev.mouse.x <= pombo_pos_x + pombo_TAMANHO &&
                    ev.mouse.y >= pombo_pos_y && ev.mouse.y <= pombo_pos_y + pombo_TAMANHO)
                {
                    gameState = VICTORY;
                }
            }
            else if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                gameState = DEFEAT;
            }
            else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                gameState = DEFEAT;
            }
            break;

        case VICTORY:
            contador = 5400;
            relogio = 90;
            al_set_audio_stream_playing(audio_theme, true);
            if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_W)
            {
                gameState = MENU;
            }
            else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                gameState = DEFEAT;
            }
            break;

        case DEFEAT:
            al_set_audio_stream_playing(audio_theme, true);
            if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_S)
            {
                gameState = MENU;
            }
            else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                gameState = DEFEAT;
            }
            break;
        }

        if (gameState == MENU || gameState == VICTORY || gameState == DEFEAT)
        {
            al_hide_mouse_cursor(display);
        }
        else
        {
            al_show_mouse_cursor(display);
        }

        if (ev.type == ALLEGRO_EVENT_TIMER)
        {
            redraw = true;
        }
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            break;
        }

        if (redraw && al_is_event_queue_empty(event_queue))
        {
            redraw = false;

            al_clear_to_color(al_map_rgb(0, 0, 0));

            switch (gameState)
            {
            case MENU:
                al_draw_bitmap(tela_inicial, 0, 0, 0);
                break;

            case PLAYING:
                al_draw_bitmap(background, 0, 0, 0);
                al_draw_bitmap(pombo, pombo_pos_x, pombo_pos_y, 0);
                char tempo_restante_str[6];
                snprintf(tempo_restante_str, 10, "%d", relogio);
                al_draw_text(font, al_map_rgb(255, 255, 255), 10, 10, 0, tempo_restante_str);
                break;

            case VICTORY:
                al_draw_bitmap(vitoria, 0, 0, 0);
                break;

            case DEFEAT:
                al_draw_bitmap(derrota, 0, 0, 0);
                break;
            }

            al_flip_display();
        }
    }

    al_destroy_bitmap(pombo);
    al_destroy_bitmap(background);
    al_destroy_bitmap(vitoria);
    al_destroy_bitmap(derrota);
    al_destroy_bitmap(tela_inicial);
    al_destroy_audio_stream(audio_theme);
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
    al_destroy_mouse_cursor(custom_cursor);

    return 0;
}
