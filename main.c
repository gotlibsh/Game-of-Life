#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define WINDOW_WIDTH    (800)
#define WINDOW_HEIGHT   (500)
#define SQUARE_SIZE     (10)

#define ERR (-1)
#define OK  (0)

typedef unsigned char   bool;
#define true            (1)
#define false           (0)

typedef enum _border_flags
{
    NO_CHECK_UP     = (1 << 0),
    NO_CHECK_DOWN   = (1 << 1),
    NO_CHECK_LEFT   = (1 << 2),
    NO_CHECK_RIGHT  = (1 << 3),
} border_flags;

typedef struct _conway_ctx_t
{
    int width;
    int height;
    int square_size;
    SDL_Color grid_color;
    SDL_Color live_color;
    SDL_Color dead_color;
    bool* table1;
    bool* table2;
    bool* active_table;
    bool initialized;
} conway_ctx_t;

conway_ctx_t g_ctx = {0};


void conway_ctx_init()
{
    g_ctx.width = WINDOW_WIDTH / SQUARE_SIZE;
    g_ctx.width += (WINDOW_WIDTH % SQUARE_SIZE) ? 1 : 0;

    g_ctx.height = WINDOW_HEIGHT / SQUARE_SIZE;
    g_ctx.height += (WINDOW_HEIGHT % SQUARE_SIZE) ? 1 : 0;

    g_ctx.square_size = SQUARE_SIZE;

    g_ctx.grid_color.r = 0xff;
    g_ctx.grid_color.a = 0xff;
    g_ctx.live_color = g_ctx.grid_color;

    g_ctx.table1 = malloc(g_ctx.width * g_ctx.height * sizeof(bool));
    g_ctx.table2 = malloc(g_ctx.width * g_ctx.height * sizeof(bool));
    g_ctx.active_table = g_ctx.table1;

    g_ctx.initialized = true;
}

void conway_ctx_deinit()
{
    if (g_ctx.table1)
    {
        free(g_ctx.table1);
    }
    if (g_ctx.table2)
    {
        free(g_ctx.table2);
    }
    SDL_memset((uint8_t*)&g_ctx, 0, sizeof(g_ctx));
}

int init_table_of_lives(bool* table_of_lives, bool* src, int src_width, int src_height)
{
    int status = ERR;


    if (table_of_lives == NULL || !g_ctx.initialized)
    {
        goto end;
    }

    // Initialize from given source
    if (src && src_width > 0 && src_height > 0)
    {
        if (src_height > g_ctx.height || src_width > g_ctx.width)
        {
            printf("Source table is bigger than destination table, source (%dx%d), destination (%dx%d)", src_height, src_width, g_ctx.height, g_ctx.width);
            goto end;
        }

        SDL_memset(table_of_lives, 0, g_ctx.width * g_ctx.height * sizeof(bool));

        for (int row = 0; row < src_height; ++row)
        {
            for (int col = 0; col < src_width; ++col)
            {
                *(table_of_lives + row * g_ctx.width + col) = *(src + row * src_width + col);
            }
        }
    }
    // Initialize randomly
    else
    {
        srand((unsigned)time(NULL));

        for (int i = 0; i < g_ctx.width * g_ctx.height; ++i)
        {
            table_of_lives[i] = rand() % 2;
        }
    }


    status = OK;

end:
    return status;
}

int resize_table(int delta)
{
    int     status          = ERR;
    int     new_square_size = 0;
    int     new_width       = 0;
    int     new_height      = 0;
    int     copy_width       = 0;
    int     copy_height      = 0;
    bool*   new_table1      = NULL;
    bool*   new_table2      = NULL;


    if (g_ctx.square_size + delta < 1 || !g_ctx.initialized)
    {
        goto end;
    }

    new_square_size = g_ctx.square_size + delta;

    new_width = WINDOW_WIDTH / new_square_size;
    new_width += (WINDOW_WIDTH % new_square_size) ? 1 : 0;

    new_height = WINDOW_HEIGHT / new_square_size;
    new_height += (WINDOW_HEIGHT % new_square_size) ? 1 : 0;

    new_table1 = malloc(new_width * new_height * sizeof(bool));

    if (new_table1 == NULL)
    {
        goto end;
    }

    new_table2 = malloc(new_width * new_height * sizeof(bool));

    if (new_table2 == NULL)
    {
        goto end;
    }

    copy_width = new_width;
    copy_height = new_height;

    if (new_width > g_ctx.width || new_height > g_ctx.height)
    {
        copy_width = g_ctx.width;
        copy_height = g_ctx.height;
    }

    SDL_memset(new_table1, 0, new_width * new_height * sizeof(bool));
    for (int row = 0; row < copy_height; ++row)
    {
        for (int col = 0; col < copy_width; ++col)
        {
            *(new_table1 + row * new_width + col) = *(g_ctx.active_table + row * g_ctx.width + col);
        }
    }
        
    free(g_ctx.table1);
    free(g_ctx.table2);

    g_ctx.table1 = new_table1;
    g_ctx.table2 = new_table2;
    g_ctx.active_table = g_ctx.table1;

    g_ctx.width = new_width;
    g_ctx.height = new_height;

    g_ctx.square_size = new_square_size;


    status = OK;

end:
    if (status != OK)
    {
        if (new_table1)
        {
            free(new_table1);
        }
        if (new_table2)
        {
            free(new_table2);
        }
    }

    return status;
}

int render_draw_grid(SDL_Renderer* renderer, SDL_Color* color)
{
    int status = ERR;


    if (renderer == NULL || color == NULL || !g_ctx.initialized)
    {
        goto end;
    }

    SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);

    for (int i = 0; i < g_ctx.width; ++i)
    {
        status = SDL_RenderDrawLine(renderer, i * g_ctx.square_size, 0, i * g_ctx.square_size, WINDOW_HEIGHT);

        if (status != OK)
        {
            printf("Failed to draw a line on the renderer with error: %s", SDL_GetError());
            goto end;
        }
    }

    for (int i = 0; i < g_ctx.height; ++i)
    {
        status = SDL_RenderDrawLine(renderer, 0, i * g_ctx.square_size, WINDOW_WIDTH, i * g_ctx.square_size);

        if (status != OK)
        {
            printf("Failed to draw a line on the renderer with error: %s", SDL_GetError());
            goto end;
        }
    }


    status = OK;

end:
    return status;
}

int get_num_of_live_neighbors(bool* table_of_lives, int row, int col, border_flags flags)
{
    int result = 0;

    
    if ((flags & NO_CHECK_LEFT) != NO_CHECK_LEFT)
    {
        result += *(table_of_lives + row * g_ctx.width + (col-1));
    }
    if ((flags & NO_CHECK_RIGHT) != NO_CHECK_RIGHT)
    {
        result += *(table_of_lives + row * g_ctx.width + (col+1));
    }
    if ((flags & NO_CHECK_UP) != NO_CHECK_UP)
    {
        result += *(table_of_lives + (row-1) * g_ctx.width + col);
    }
    if ((flags & NO_CHECK_DOWN) != NO_CHECK_DOWN)
    {
        result += *(table_of_lives + (row+1) * g_ctx.width + col);
    }
    if (!(flags & (NO_CHECK_LEFT | NO_CHECK_UP)))
    {
        result += *(table_of_lives + (row-1) * g_ctx.width + (col-1));
    }
    if (!(flags & (NO_CHECK_LEFT | NO_CHECK_DOWN)))
    {
        result += *(table_of_lives + (row+1) * g_ctx.width + (col-1));
    }
    if (!(flags & (NO_CHECK_RIGHT | NO_CHECK_UP)))
    {
        result += *(table_of_lives + (row-1) * g_ctx.width + (col+1));
    }
    if (!(flags & (NO_CHECK_RIGHT | NO_CHECK_DOWN)))
    {
        result += *(table_of_lives + (row+1) * g_ctx.width + (col+1));
    }

    return result;
}

int update_table_of_lives()
{
    int             status                  = ERR;
    int             num_of_live_neighbors   = 0;
    border_flags    border_flags            = 0;
    bool*           cur_table               = NULL;
    bool*           next_table              = NULL; 

    
    if (!g_ctx.initialized)
    {
        goto end;
    }

    cur_table = g_ctx.active_table;
    if (g_ctx.active_table == g_ctx.table1)
    {
        next_table = g_ctx.table2;
    }
    else
    {
        next_table = g_ctx.table1;
    }

    for (int row = 0; row < g_ctx.height; ++row)
    {
        for (int col = 0; col < g_ctx.width; ++col)
        {
            border_flags = 0;

            if (row == 0)
            {
                border_flags |= NO_CHECK_UP;
            }
            if (row == g_ctx.height - 1)
            {
                border_flags |= NO_CHECK_DOWN;
            }
            if (col == 0)
            {
                border_flags |= NO_CHECK_LEFT;
            }
            if (col == g_ctx.width - 1)
            {
                border_flags |= NO_CHECK_RIGHT;
            }

            num_of_live_neighbors = get_num_of_live_neighbors(cur_table, row, col, border_flags);

            // Rule 1: live cell with 2 or 3 live neighbors survives
            if (*(cur_table + row * g_ctx.width + col) && (num_of_live_neighbors == 2 || num_of_live_neighbors == 3))
            {
                *(next_table + row * g_ctx.width + col) = true;
                continue;
            }

            // Rule 2: dead cell with 3 live neighbors becomes live
            if (*(cur_table + row * g_ctx.width + col) == false && num_of_live_neighbors == 3)
            {
                *(next_table + row * g_ctx.width + col) = true;
                continue;
            }

            // Rule 3 & 4: live and dead cells that don't apply to the previous rules become dead
            *(next_table + row * g_ctx.width + col) = false;
        }
    }

    g_ctx.active_table = next_table;
    status = OK;

end:
    return status;
}

int render_draw_table_of_lives(SDL_Renderer* renderer, bool* table_of_lives)
{
    int         status  = ERR;
    SDL_Rect    dest    = {0};


    if (renderer == NULL || table_of_lives == NULL || !g_ctx.initialized)
    {
        goto end;
    }

    dest.w = g_ctx.square_size;
    dest.h = g_ctx.square_size;

    for (int row = 0; row < g_ctx.height; ++row)
    {
        dest.y = row * g_ctx.square_size;

        for (int col = 0; col < g_ctx.width; ++col)
        {
            dest.x = col * g_ctx.square_size;

            if (*(table_of_lives + row * g_ctx.width + col))
            {
                SDL_SetRenderDrawColor(renderer, g_ctx.live_color.r, g_ctx.live_color.g, g_ctx.live_color.b, g_ctx.live_color.a);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, g_ctx.dead_color.r, g_ctx.dead_color.g, g_ctx.dead_color.b, g_ctx.dead_color.a);
            }
            
            SDL_RenderFillRect(renderer, &dest);
        }
    }


    status = OK;

end:
    return status;
}

int main(int argc, char* argv[])
{
    int             status          = ERR;
    bool            close_requested = false;
    SDL_Window*     window          = NULL;
    SDL_Renderer*   renderer        = NULL;


    conway_ctx_init();

    bool gilder_gun[] =
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,
        0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,
        0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };
    status = init_table_of_lives(g_ctx.active_table, gilder_gun, 38, 11);

    if (status != OK)
    {
        goto end;
    }

    status = SDL_Init(SDL_INIT_VIDEO);

    if (status != OK)
    {
        printf("Failed to initialize SDL with error: %s", SDL_GetError());
        goto end;
    }

    window = SDL_CreateWindow("Conway's Game of Life",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_WIDTH,
                                          WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);

    if (window == NULL)
    {
        printf("Failed to create SDL window with error: %s", SDL_GetError());
        goto end;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (renderer == NULL)
    {
        printf("Failed to create a renderer with error: %s", SDL_GetError());
        goto end;
    }

    while (!close_requested)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                close_requested = true;
            }
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                case SDL_SCANCODE_SPACE:
                    status = update_table_of_lives();

                    if (status != OK)
                    {
                        goto end;
                    }

                    break;
                }
            }
            if (event.type == SDL_MOUSEMOTION)
            {
                int x, y;
                int button_state = SDL_GetMouseState(&x, &y);

                x /= g_ctx.square_size;
                y /= g_ctx.square_size;

                if (button_state & SDL_BUTTON(SDL_BUTTON_LEFT))
                {
                    *(g_ctx.active_table + y * g_ctx.width + x) = true;
                }
                if (button_state & SDL_BUTTON(SDL_BUTTON_RIGHT))
                {
                    *(g_ctx.active_table + y * g_ctx.width + x) = false;
                }
            }
            if (event.type == SDL_MOUSEWHEEL)
            {
                resize_table(event.wheel.y);
            }

            status = SDL_RenderClear(renderer);

            if (status != OK)
            {
                printf("Failed to clear the renderer with error: %s", SDL_GetError());
                goto end;
            }

            status = render_draw_table_of_lives(renderer, g_ctx.active_table);

            if (status != OK)
            {
                goto end;
            }

            SDL_RenderPresent(renderer);
        }
    }


end:
    SDL_DestroyWindow(NULL);
    SDL_Quit();
    conway_ctx_deinit();

    return 0;    
}