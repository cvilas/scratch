//g++ -o sdlplot sdlplot.cpp -lSDL3 -lSDL3_ttf

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_POINTS     400
#define GRID_SPACING    50
#define NUM_TICKS        5
#define TICK_LEN         6
#define FONT_SIZE       11   /* TTF font point size */
#define MARGIN_L        65
#define MARGIN_T        30
#define MARGIN_R        20
#define MARGIN_B        70
#define MIN_X_VISIBLE   10
#define ZOOM_IN_FAC   0.80f
#define ZOOM_OUT_FAC  1.25f
#define X_GRID_DIVS      6   /* target vertical grid lines        */
#define Y_GRID_DIVS      5   /* target horizontal grid lines      */

typedef struct {
    float* values;
    int    count, capacity, head;
    int    total_count;  /* never resets – used for scrolling x labels */
    float  min_val, max_val;
} TimeSeries;

typedef struct {
    float x, y, w, h;
    bool  dragging;
    float drag_dx, drag_dy;
} Legend;

typedef struct {
    SDL_Window*   window;
    SDL_Renderer* renderer;
    TimeSeries    series;
    int           plot_x, plot_y, plot_width, plot_height;
    float         x_scale;
    Legend        legend;
    /* Zoom state */
    int   x_visible;   /* samples to show; 0 = all */
    bool  y_manual;    /* user has set a custom Y range */
    float vy_min, vy_max;
    /* Derived per-frame */
    int   view_start, view_n;
    /* User-specified labels */
    char  x_label[64];
    char  y_label[64];
    char  series_name[64];
    /* TTF font */
    TTF_Font* font;
} PlotContext;

/* ── TimeSeries helpers ─────────────────────────────────────────────────── */

void init_timeseries(TimeSeries* ts, int capacity) {
    ts->values      = (float*)malloc(capacity * sizeof(float));
    ts->count       = 0;
    ts->capacity    = capacity;
    ts->head        = 0;
    ts->total_count = 0;
    ts->min_val     = INFINITY;
    ts->max_val     = -INFINITY;
}

void add_datapoint(TimeSeries* ts, float value) {
    ts->values[ts->head] = value;
    ts->head = (ts->head + 1) % ts->capacity;
    if (ts->count < ts->capacity) ts->count++;
    ts->total_count++;

    if (value < ts->min_val) ts->min_val = value;
    if (value > ts->max_val) ts->max_val = value;

    if (ts->count % 50 == 0) {
        ts->min_val = INFINITY;
        ts->max_val = -INFINITY;
        for (int i = 0; i < ts->count; i++) {
            int idx = (ts->head - ts->count + i + ts->capacity) % ts->capacity;
            if (ts->values[idx] < ts->min_val) ts->min_val = ts->values[idx];
            if (ts->values[idx] > ts->max_val) ts->max_val = ts->values[idx];
        }
    }
}

float get_value(TimeSeries* ts, int index) {
    if (index < 0 || index >= ts->count) return 0;
    return ts->values[(ts->head - ts->count + index + ts->capacity) % ts->capacity];
}

/* Look up by global (ever-increasing) sample index */
float get_value_global(TimeSeries* ts, int g) {
    int local = ts->count + g - ts->total_count;
    return get_value(ts, local);
}

/* ── Layout ─────────────────────────────────────────────────────────────── */

void recalc_plot_area(PlotContext* ctx) {
    int w, h;
    SDL_GetWindowSize(ctx->window, &w, &h);
    ctx->plot_x      = MARGIN_L;
    ctx->plot_y      = MARGIN_T;
    ctx->plot_width  = w - MARGIN_L - MARGIN_R;
    ctx->plot_height = h - MARGIN_T - MARGIN_B;
}

/* Called once per frame before drawing to derive view_start/view_n/x_scale/vy */
void update_view(PlotContext* ctx) {
    TimeSeries* ts = &ctx->series;

    int xv = (ctx->x_visible > 0) ? ctx->x_visible : ts->count;
    if (xv > ts->count) xv = ts->count;
    if (xv < 2)         xv = 2;
    ctx->view_n     = xv;
    /* Use total_count so view_start keeps growing beyond capacity */
    ctx->view_start = ts->total_count - xv;
    if (ctx->view_start < 0) ctx->view_start = 0;
    ctx->x_scale = (ctx->view_n > 1)
                   ? (float)ctx->plot_width / (ctx->view_n - 1)
                   : 1.0f;

    if (!ctx->y_manual) {
        ctx->vy_min = ts->min_val;
        ctx->vy_max = ts->max_val;
    }
    if (ctx->vy_max - ctx->vy_min < 1e-6f) {
        ctx->vy_min -= 1.0f;
        ctx->vy_max += 1.0f;
    }
}

static bool in_plot(PlotContext* ctx, float mx, float my) {
    return mx >= ctx->plot_x && mx <= ctx->plot_x + ctx->plot_width &&
           my >= ctx->plot_y && my <= ctx->plot_y + ctx->plot_height;
}

/* ── Nice step helper ───────────────────────────────────────────────────── */

/* Returns a "human-friendly" step size for ~n divisions of range */
static float nice_step(float range, int n) {
    if (range <= 0.0f || n <= 0) return 1.0f;
    float rough = range / n;
    float mag   = powf(10.0f, floorf(log10f(rough)));
    float norm  = rough / mag;
    float step  = (norm < 1.5f) ? 1.0f * mag :
                  (norm < 3.5f) ? 2.0f * mag :
                  (norm < 7.5f) ? 5.0f * mag : 10.0f * mag;
    return step;
}

/* ── TTF text rendering helpers ─────────────────────────────────────────── */

static void render_text(SDL_Renderer* r, TTF_Font* font,
                        const char* text, float x, float y, SDL_Color col) {
    SDL_Surface* surf = TTF_RenderText_Blended(font, text, 0, col);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FRect dst = { x, y, (float)surf->w, (float)surf->h };
    SDL_DestroySurface(surf);
    if (!tex) return;
    SDL_RenderTexture(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

/* Renders text rotated -90° (reads bottom-to-top), centred at (cx, cy) */
static void render_text_vertical(SDL_Renderer* r, TTF_Font* font,
                                  const char* text, float cx, float cy,
                                  SDL_Color col) {
    SDL_Surface* surf = TTF_RenderText_Blended(font, text, 0, col);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    float tw = (float)surf->w, th = (float)surf->h;
    SDL_DestroySurface(surf);
    if (!tex) return;
    /* Place at natural size centred on (cx, cy), then rotate -90° around that centre */
    SDL_FRect dst = { cx - tw / 2.0f, cy - th / 2.0f, tw, th };
    SDL_RenderTextureRotated(r, tex, NULL, &dst, -90.0, NULL, SDL_FLIP_NONE);
    SDL_DestroyTexture(tex);
}

void draw_grid(PlotContext* ctx) {
    if (ctx->view_n < 2) return;
    SDL_SetRenderDrawColor(ctx->renderer, 48, 48, 48, 255);

    /* Vertical lines – anchored to fixed sample indices, scroll with data */
    float x_step   = nice_step((float)ctx->view_n, X_GRID_DIVS);
    float first_gx = ceilf((float)ctx->view_start / x_step) * x_step;
    for (float gx = first_gx; gx <= ctx->view_start + ctx->view_n; gx += x_step) {
        int sx = ctx->plot_x + (int)((gx - ctx->view_start) * ctx->x_scale);
        if (sx < ctx->plot_x || sx > ctx->plot_x + ctx->plot_width) continue;
        SDL_RenderLine(ctx->renderer, sx, ctx->plot_y, sx, ctx->plot_y + ctx->plot_height);
    }

    /* Horizontal lines – anchored to fixed data values, scroll when Y zooms */
    float yrange   = ctx->vy_max - ctx->vy_min;
    float y_step   = nice_step(yrange, Y_GRID_DIVS);
    float first_gy = ceilf(ctx->vy_min / y_step) * y_step;
    for (float gy = first_gy; gy <= ctx->vy_max + y_step * 0.01f; gy += y_step) {
        float norm = (gy - ctx->vy_min) / yrange;
        if (norm < 0.0f || norm > 1.0f) continue;
        int sy = ctx->plot_y + ctx->plot_height - (int)(norm * ctx->plot_height);
        SDL_RenderLine(ctx->renderer, ctx->plot_x, sy, ctx->plot_x + ctx->plot_width, sy);
    }
}

void draw_axes(PlotContext* ctx) {
    SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 255);
    SDL_RenderLine(ctx->renderer, ctx->plot_x, ctx->plot_y,
                   ctx->plot_x, ctx->plot_y + ctx->plot_height);
    SDL_RenderLine(ctx->renderer, ctx->plot_x, ctx->plot_y + ctx->plot_height,
                   ctx->plot_x + ctx->plot_width, ctx->plot_y + ctx->plot_height);
}

/* ── Coordinate conversion ──────────────────────────────────────────────── */

void data_to_screen(PlotContext* ctx, int abs_idx, float data_y,
                    int* sx, int* sy) {
    int rel = abs_idx - ctx->view_start;
    *sx = ctx->plot_x + (int)(rel * ctx->x_scale);
    float range = ctx->vy_max - ctx->vy_min;
    float norm  = (data_y - ctx->vy_min) / range;
    *sy = ctx->plot_y + ctx->plot_height - (int)(norm * ctx->plot_height);
}

float screen_to_data_y(PlotContext* ctx, float sy) {
    float norm = 1.0f - (sy - ctx->plot_y) / ctx->plot_height;
    return ctx->vy_min + norm * (ctx->vy_max - ctx->vy_min);
}

void draw_timeseries(PlotContext* ctx) {
    if (ctx->view_n < 2) return;

    SDL_SetRenderDrawColor(ctx->renderer, 0, 255, 0, 255);
    int end = ctx->view_start + ctx->view_n - 1;
    for (int i = ctx->view_start; i < end; i++) {
        int x1, y1, x2, y2;
        data_to_screen(ctx, i,   get_value_global(&ctx->series, i),   &x1, &y1);
        data_to_screen(ctx, i+1, get_value_global(&ctx->series, i+1), &x2, &y2);
        SDL_RenderLine(ctx->renderer, x1, y1, x2, y2);
    }

    /* Red dot at latest visible sample */
    int x, y;
    data_to_screen(ctx, end, get_value_global(&ctx->series, end), &x, &y);
    SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 0, 255);
    for (int i = -2; i <= 2; i++)
        for (int j = -2; j <= 2; j++)
            if (i*i + j*j <= 4)
                SDL_RenderPoint(ctx->renderer, x + i, y + j);
}

/* ── Tick marks and axis labels ─────────────────────────────────────────── */

void draw_axis_labels(PlotContext* ctx) {
    int fh = TTF_GetFontHeight(ctx->font);

    /* ---- Y ticks: same positions as horizontal grid lines ---- */
    float yrange   = ctx->vy_max - ctx->vy_min;
    float y_step   = nice_step(yrange, Y_GRID_DIVS);
    float first_gy = ceilf(ctx->vy_min / y_step) * y_step;

    for (float gy = first_gy; gy <= ctx->vy_max + y_step * 0.01f; gy += y_step) {
        float norm = (gy - ctx->vy_min) / yrange;
        if (norm < 0.0f || norm > 1.0f) continue;
        int sy = ctx->plot_y + ctx->plot_height - (int)(norm * ctx->plot_height);

        SDL_SetRenderDrawColor(ctx->renderer, 200, 200, 200, 255);
        SDL_RenderLine(ctx->renderer, ctx->plot_x - TICK_LEN, sy, ctx->plot_x, sy);

        char lbl[24];
        snprintf(lbl, sizeof(lbl), "%.4g", gy);
        int tw = 0, th = 0;
        TTF_GetStringSize(ctx->font, lbl, 0, &tw, &th);
        SDL_Color col = {200, 200, 200, 255};
        render_text(ctx->renderer, ctx->font, lbl,
                    ctx->plot_x - TICK_LEN - tw - 2,
                    sy - th / 2.0f, col);
    }

    /* Y axis title – rotated, centred in the left margin */
    const char* ytitle = ctx->y_label[0] ? ctx->y_label : "Y";
    SDL_Color ytcol = {160, 160, 255, 255};
    render_text_vertical(ctx->renderer, ctx->font, ytitle,
                         fh / 2.0f + 2,
                         ctx->plot_y + ctx->plot_height / 2.0f, ytcol);

    if (ctx->view_n < 2) return;

    /* ---- X ticks: same positions as vertical grid lines ---- */
    int   base_y   = ctx->plot_y + ctx->plot_height;
    float x_step   = nice_step((float)ctx->view_n, X_GRID_DIVS);
    float first_gx = ceilf((float)ctx->view_start / x_step) * x_step;

    for (float gx = first_gx; gx <= ctx->view_start + ctx->view_n; gx += x_step) {
        int sx = ctx->plot_x + (int)((gx - ctx->view_start) * ctx->x_scale);
        if (sx < ctx->plot_x || sx > ctx->plot_x + ctx->plot_width) continue;

        SDL_SetRenderDrawColor(ctx->renderer, 200, 200, 200, 255);
        SDL_RenderLine(ctx->renderer, sx, base_y, sx, base_y + TICK_LEN);

        char lbl[16];
        snprintf(lbl, sizeof(lbl), "%.0f", gx);
        int tw = 0, th = 0;
        TTF_GetStringSize(ctx->font, lbl, 0, &tw, &th);
        SDL_Color col = {200, 200, 200, 255};
        render_text(ctx->renderer, ctx->font, lbl,
                    sx - tw / 2.0f,
                    base_y + TICK_LEN + 2, col);
    }

    /* X axis title */
    const char* xtitle = ctx->x_label[0] ? ctx->x_label : "X";
    {
        int tw = 0, th = 0;
        TTF_GetStringSize(ctx->font, xtitle, 0, &tw, &th);
        SDL_Color col = {160, 160, 255, 255};
        render_text(ctx->renderer, ctx->font, xtitle,
                    ctx->plot_x + ctx->plot_width / 2.0f - tw / 2.0f,
                    base_y + TICK_LEN + fh + 6, col);
    }

    /* Zoom hint */
    if (ctx->x_visible > 0 || ctx->y_manual) {
        SDL_Color col = {120, 120, 120, 255};
        render_text(ctx->renderer, ctx->font, "dbl-click: reset zoom",
                    ctx->plot_x + 2, ctx->plot_y + 2, col);
    }
}

/* ── Movable legend ─────────────────────────────────────────────────────── */

static bool legend_hit(Legend* leg, float mx, float my) {
    return mx >= leg->x && mx <= leg->x + leg->w &&
           my >= leg->y && my <= leg->y + leg->h;
}

void draw_legend(PlotContext* ctx) {
    Legend* leg = &ctx->legend;

    SDL_FRect bg = { leg->x, leg->y, leg->w, leg->h };

    /* Background */
    SDL_SetRenderDrawColor(ctx->renderer, 20, 20, 20, 210);
    SDL_RenderFillRect(ctx->renderer, &bg);

    /* Border */
    SDL_SetRenderDrawColor(ctx->renderer, 180, 180, 180, 255);
    SDL_RenderRect(ctx->renderer, &bg);

    /* Colour swatch: green line + red dot (mirrors the series style) */
    float sy = leg->y + leg->h / 2.0f;
    SDL_SetRenderDrawColor(ctx->renderer, 0, 255, 0, 255);
    SDL_RenderLine(ctx->renderer, leg->x + 6, sy, leg->x + 22, sy);
    SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 0, 255);
    for (int i = -2; i <= 2; i++)
        for (int j = -2; j <= 2; j++)
            if (i*i + j*j <= 4)
                SDL_RenderPoint(ctx->renderer, leg->x + 22 + i, sy + j);

    /* Series name */
    const char* name = ctx->series_name[0] ? ctx->series_name : "Series 1";
    int tw = 0, th = 0;
    TTF_GetStringSize(ctx->font, name, 0, &tw, &th);
    SDL_Color col = {255, 255, 255, 255};
    render_text(ctx->renderer, ctx->font, name,
                leg->x + 28,
                leg->y + leg->h / 2.0f - th / 2.0f, col);
}

/* ── Zoom helpers ───────────────────────────────────────────────────────── */

void zoom_x(PlotContext* ctx, float factor) {
    int cur = (ctx->x_visible > 0) ? ctx->x_visible : ctx->series.count;
    int nxt = (int)(cur * factor);
    if (nxt < MIN_X_VISIBLE)      nxt = MIN_X_VISIBLE;
    if (nxt >= ctx->series.count) { ctx->x_visible = 0; return; } /* all = unzoomed */
    ctx->x_visible = nxt;
}

void zoom_y(PlotContext* ctx, float factor, float mouse_y) {
    float pivot   = screen_to_data_y(ctx, mouse_y);
    float new_rng = (ctx->vy_max - ctx->vy_min) * factor;
    float lo_frac = (pivot - ctx->vy_min) / (ctx->vy_max - ctx->vy_min);
    ctx->vy_min   = pivot - lo_frac * new_rng;
    ctx->vy_max   = ctx->vy_min + new_rng;
    ctx->y_manual = true;
}

void reset_zoom(PlotContext* ctx) {
    ctx->x_visible = 0;
    ctx->y_manual  = false;
}

/* ── Sample data generator ──────────────────────────────────────────────── */

float generate_sample_data() {
    static float t = 0;
    t += 0.1f;
    return sinf(t) * 50 + cosf(t * 0.3f) * 30 + sinf(t * 2) * 10;
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return 1;
    }

    PlotContext ctx = {0};

    ctx.window = SDL_CreateWindow("Real-time Time Series Plot",
                                  800, 600, SDL_WINDOW_RESIZABLE);
    if (!ctx.window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    ctx.renderer = SDL_CreateRenderer(ctx.window, NULL);
    if (!ctx.renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(ctx.window);
        SDL_Quit();
        return 1;
    }

    if (!TTF_Init()) {
        printf("TTF_Init failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(ctx.renderer);
        SDL_DestroyWindow(ctx.window);
        SDL_Quit();
        return 1;
    }
    ctx.font = TTF_OpenFont("/usr/share/fonts/google-droid-sans-fonts/DroidSans.ttf", FONT_SIZE);
    if (!ctx.font) {
        printf("TTF_OpenFont failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_DestroyRenderer(ctx.renderer);
        SDL_DestroyWindow(ctx.window);
        SDL_Quit();
        return 1;
    }

    recalc_plot_area(&ctx);
    init_timeseries(&ctx.series, MAX_POINTS);

    /* ── Set axis labels and series name here ─────────────────────────── */
    strncpy(ctx.x_label,    "Time (samples)", sizeof(ctx.x_label) - 1);
    strncpy(ctx.y_label,    "Amplitude",      sizeof(ctx.y_label) - 1);
    strncpy(ctx.series_name,"Signal",         sizeof(ctx.series_name) - 1);

    /* Legend – size based on font metrics, initial position top-right of plot */
    {
        const char* lname = ctx.series_name[0] ? ctx.series_name : "Series 1";
        int tw = 0, th = 0;
        TTF_GetStringSize(ctx.font, lname, 0, &tw, &th);
        ctx.legend.w = 28 + tw + 8;
        ctx.legend.h = th + 8;
    }
    ctx.legend.x = ctx.plot_x + ctx.plot_width - ctx.legend.w - 4;
    ctx.legend.y = ctx.plot_y + 10;

    int running = 1;
    SDL_Event e;
    Uint32 last_update = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT:
                running = 0;
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                recalc_plot_area(&ctx);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    float mx = e.button.x, my = e.button.y;
                    if (legend_hit(&ctx.legend, mx, my)) {
                        ctx.legend.dragging = true;
                        ctx.legend.drag_dx  = mx - ctx.legend.x;
                        ctx.legend.drag_dy  = my - ctx.legend.y;
                    } else if (e.button.clicks == 2 && in_plot(&ctx, mx, my)) {
                        reset_zoom(&ctx);
                    }
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (ctx.legend.dragging) {
                    ctx.legend.x = e.motion.x - ctx.legend.drag_dx;
                    ctx.legend.y = e.motion.y - ctx.legend.drag_dy;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (e.button.button == SDL_BUTTON_LEFT)
                    ctx.legend.dragging = false;
                break;

            case SDL_EVENT_MOUSE_WHEEL: {
                float mx = 0, my = 0;
                SDL_GetMouseState(&mx, &my);
                if (!in_plot(&ctx, mx, my)) break;
                bool ctrl = (SDL_GetModState() & SDL_KMOD_CTRL) != 0;
                float fac = (e.wheel.y > 0) ? ZOOM_IN_FAC : ZOOM_OUT_FAC;
                if (ctrl)
                    zoom_y(&ctx, fac, my);
                else
                    zoom_x(&ctx, fac);
                break;
            }
            }
        }

        /* Add new data point every 50 ms */
        Uint32 now = SDL_GetTicks();
        if (now - last_update > 50) {
            add_datapoint(&ctx.series, generate_sample_data());
            last_update = now;
        }

        update_view(&ctx);

        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255);
        SDL_RenderClear(ctx.renderer);

        draw_grid(&ctx);
        draw_axes(&ctx);
        draw_timeseries(&ctx);
        draw_axis_labels(&ctx);
        draw_legend(&ctx);

        SDL_RenderPresent(ctx.renderer);
        SDL_Delay(16); /* ~60 FPS */
    }

    free(ctx.series.values);
    TTF_CloseFont(ctx.font);
    TTF_Quit();
    SDL_DestroyRenderer(ctx.renderer);
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();
    return 0;
}
