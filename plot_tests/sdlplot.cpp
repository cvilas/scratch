//g++ -o sdlplot sdlplot.cpp -lSDL3

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MAX_POINTS 400
#define GRID_SPACING 50

typedef struct {
    float* values;
    int count;
    int capacity;
    int head; // circular buffer head
    float min_val, max_val;
} TimeSeries;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TimeSeries series;
    int plot_x, plot_y, plot_width, plot_height;
    float x_scale, y_scale;
} PlotContext;

// Initialize time series with circular buffer
void init_timeseries(TimeSeries* ts, int capacity) {
    ts->values = (float*)(malloc(capacity * sizeof(float)));
    ts->count = 0;
    ts->capacity = capacity;
    ts->head = 0;
    ts->min_val = INFINITY;
    ts->max_val = -INFINITY;
}

// Add new data point (circular buffer)
void add_datapoint(TimeSeries* ts, float value) {
    ts->values[ts->head] = value;
    ts->head = (ts->head + 1) % ts->capacity;

    if (ts->count < ts->capacity) {
        ts->count++;
    }

    // Update min/max for auto-scaling
    if (value < ts->min_val) ts->min_val = value;
    if (value > ts->max_val) ts->max_val = value;

    // Recalculate min/max every so often to handle changing ranges
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

// Get value at index (handles circular buffer)
float get_value(TimeSeries* ts, int index) {
    if (index >= ts->count) return 0;
    int actual_idx = (ts->head - ts->count + index + ts->capacity) % ts->capacity;
    return ts->values[actual_idx];
}

// Draw grid lines
void draw_grid(PlotContext* ctx) {
    SDL_SetRenderDrawColor(ctx->renderer, 64, 64, 64, 255);

    // Vertical grid lines
    for (int x = ctx->plot_x; x <= ctx->plot_x + ctx->plot_width; x += GRID_SPACING) {
        SDL_RenderLine(ctx->renderer, x, ctx->plot_y, x, ctx->plot_y + ctx->plot_height);
    }

    // Horizontal grid lines
    for (int y = ctx->plot_y; y <= ctx->plot_y + ctx->plot_height; y += GRID_SPACING) {
        SDL_RenderLine(ctx->renderer, ctx->plot_x, y, ctx->plot_x + ctx->plot_width, y);
    }
}

// Draw axes
void draw_axes(PlotContext* ctx) {
    SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 255);

    // Y-axis
    SDL_RenderLine(ctx->renderer, ctx->plot_x, ctx->plot_y,
                       ctx->plot_x, ctx->plot_y + ctx->plot_height);

    // X-axis
    SDL_RenderLine(ctx->renderer, ctx->plot_x, ctx->plot_y + ctx->plot_height,
                       ctx->plot_x + ctx->plot_width, ctx->plot_y + ctx->plot_height);
}

// Convert data coordinates to screen coordinates
void data_to_screen(PlotContext* ctx, int data_x, float data_y, int* screen_x, int* screen_y) {
    *screen_x = ctx->plot_x + (int)(data_x * ctx->x_scale);

    float range = ctx->series.max_val - ctx->series.min_val;
    if (range == 0) range = 1; // Avoid division by zero

    float normalized_y = (data_y - ctx->series.min_val) / range;
    *screen_y = ctx->plot_y + ctx->plot_height - (int)(normalized_y * ctx->plot_height);
}

// Draw the time series line
void draw_timeseries(PlotContext* ctx) {
    if (ctx->series.count < 2) return;

    SDL_SetRenderDrawColor(ctx->renderer, 0, 255, 0, 255); // Green line

    ctx->x_scale = (float)ctx->plot_width / (ctx->series.count - 1);

    for (int i = 0; i < ctx->series.count - 1; i++) {
        int x1, y1, x2, y2;

        data_to_screen(ctx, i, get_value(&ctx->series, i), &x1, &y1);
        data_to_screen(ctx, i + 1, get_value(&ctx->series, i + 1), &x2, &y2);

        SDL_RenderLine(ctx->renderer, x1, y1, x2, y2);
    }

    // Draw current point as a circle
    if (ctx->series.count > 0) {
        int x, y;
        data_to_screen(ctx, ctx->series.count - 1,
                       get_value(&ctx->series, ctx->series.count - 1), &x, &y);

        SDL_SetRenderDrawColor(ctx->renderer, 255, 0, 0, 255); // Red dot
        for (int i = -2; i <= 2; i++) {
            for (int j = -2; j <= 2; j++) {
                if (i*i + j*j <= 4) {
                    SDL_RenderPoint(ctx->renderer, x + i, y + j);
                }
            }
        }
    }
}

// Generate sample data (replace with your data source)
float generate_sample_data() {
    static float t = 0;
    t += 0.1f;
    return sin(t) * 50 + cos(t * 0.3f) * 30 + sin(t * 2) * 10;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return 1;
    }

    PlotContext ctx = {0};

    ctx.window = SDL_CreateWindow("Real-time Time Series Plot",
                                  WINDOW_WIDTH, WINDOW_HEIGHT, 0);

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

    // Set up plot area
    ctx.plot_x = 50;
    ctx.plot_y = 50;
    ctx.plot_width = WINDOW_WIDTH - 100;
    ctx.plot_height = WINDOW_HEIGHT - 100;

    init_timeseries(&ctx.series, MAX_POINTS);

    int running = 1;
    SDL_Event e;
    Uint32 last_update = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = 0;
            }
        }

        // Add new data point every 50ms
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_update > 50) {
            float new_value = generate_sample_data();
            add_datapoint(&ctx.series, new_value);
            last_update = current_time;
        }

        // Clear screen
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255);
        SDL_RenderClear(ctx.renderer);

        // Draw plot elements
        draw_grid(&ctx);
        draw_axes(&ctx);
        draw_timeseries(&ctx);

        SDL_RenderPresent(ctx.renderer);

        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    free(ctx.series.values);
    SDL_DestroyRenderer(ctx.renderer);
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();

    return 0;
}
