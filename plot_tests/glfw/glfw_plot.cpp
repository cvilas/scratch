#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <deque>
#include <algorithm>
#include <iomanip>
#include <sstream>

// Structure to hold plot configuration
struct PlotConfig {
    float xMin, xMax;
    float yMin, yMax;
    float windowWidth;  // Time window width in seconds
    bool autoScale;

    PlotConfig() : xMin(0), xMax(10), yMin(-1.5f), yMax(1.5f),
    windowWidth(10.0f), autoScale(true) {}
};

// Structure to hold time series data
struct TimeSeries {
    std::deque<float> timeData;
    std::deque<float> valueData;
    float r, g, b;  // Color
    float lineWidth;
    size_t maxPoints;

    TimeSeries(float red = 1.0f, float green = 1.0f, float blue = 1.0f,
               float width = 2.0f, size_t maxPts = 10000)
    : r(red), g(green), b(blue), lineWidth(width), maxPoints(maxPts) {}

    void addPoint(float time, float value) {
        timeData.push_back(time);
        valueData.push_back(value);

        // Keep only recent points
        while (timeData.size() > maxPoints) {
            timeData. pop_front();
            valueData.pop_front();
        }
    }

    void clear() {
        timeData. clear();
        valueData.clear();
    }
};

// Function to convert normalized coordinates to screen coordinates
void normalizedToScreen(float xNorm, float yNorm, float aspect, float& xOut, float& yOut) {
    if (aspect >= 1.0f) {
        // Window is wider than tall
        xOut = xNorm / aspect;
        yOut = yNorm;
    } else {
        // Window is taller than wide
        xOut = xNorm;
        yOut = yNorm * aspect;
    }
}

// Function to draw a line with aspect ratio correction
void drawLine(float x1, float y1, float x2, float y2, float aspect) {
    float sx1, sy1, sx2, sy2;
    normalizedToScreen(x1, y1, aspect, sx1, sy1);
    normalizedToScreen(x2, y2, aspect, sx2, sy2);

    glBegin(GL_LINES);
    glVertex2f(sx1, sy1);
    glVertex2f(sx2, sy2);
    glEnd();
}

// Function to calculate nice grid spacing
float calculateNiceGridSpacing(float range, int targetDivisions) {
    float roughSpacing = range / targetDivisions;

    // Find the magnitude (power of 10)
    float magnitude = std::pow(10.0f, std:: floor(std::log10(roughSpacing)));

    // Normalize to range [1, 10)
    float normalized = roughSpacing / magnitude;

    // Choose nice number:  1, 2, 5, or 10
    float niceNormalized;
    if (normalized <= 1.5f) {
        niceNormalized = 1.0f;
    } else if (normalized <= 3.0f) {
        niceNormalized = 2.0f;
    } else if (normalized <= 7.0f) {
        niceNormalized = 5.0f;
    } else {
        niceNormalized = 10.0f;
    }

    return niceNormalized * magnitude;
}

// Function to draw axes with aspect ratio correction
void drawAxes(float aspect, const PlotConfig& config) {
    glColor3f(0.7f, 0.7f, 0.7f);
    glLineWidth(2.0f);

    // Only draw axes if zero is in range
    if (config.yMin <= 0 && config.yMax >= 0) {
        // X-axis at y=0
        float yPos = -2.0f * (0 - config.yMin) / (config.yMax - config.yMin) + 1.0f;
        drawLine(-1.0f, yPos, 1.0f, yPos, aspect);
    }

    if (config.xMin <= 0 && config.xMax >= 0) {
        // Y-axis at x=0
        float xPos = 2.0f * (0 - config.xMin) / (config.xMax - config.xMin) - 1.0f;
        drawLine(xPos, -1.0f, xPos, 1.0f, aspect);
    }
}

// Function to draw a moving grid with aspect ratio correction
void drawGrid(float aspect, const PlotConfig& config) {
    glLineWidth(1.0f);

    // Calculate nice grid spacing
    float xRange = config.xMax - config. xMin;
    float yRange = config.yMax - config.yMin;

    float xSpacing = calculateNiceGridSpacing(xRange, 10);
    float ySpacing = calculateNiceGridSpacing(yRange, 8);

    // Vertical grid lines (time axis)
    // Find the first grid line position
    float xStart = std::ceil(config.xMin / xSpacing) * xSpacing;

    glColor3f(0.2f, 0.2f, 0.25f);
    for (float x = xStart; x <= config.xMax; x += xSpacing) {
        // Convert to normalized coordinates
        float xNorm = 2.0f * (x - config.xMin) / (config.xMax - config.xMin) - 1.0f;

        // Check if this is a major grid line (every 5th or at nice round numbers)
        bool isMajor = (std::abs(std::fmod(x, xSpacing * 5.0f)) < xSpacing * 0.01f);

        if (isMajor) {
            glColor3f(0.3f, 0.3f, 0.35f);
            glLineWidth(1.5f);
        } else {
            glColor3f(0.2f, 0.2f, 0.25f);
            glLineWidth(1.0f);
        }

        drawLine(xNorm, -1.0f, xNorm, 1.0f, aspect);
    }

    // Horizontal grid lines (value axis)
    float yStart = std::ceil(config.yMin / ySpacing) * ySpacing;

    for (float y = yStart; y <= config.yMax; y += ySpacing) {
        // Convert to normalized coordinates
        float yNorm = 2.0f * (y - config.yMin) / (config.yMax - config.yMin) - 1.0f;

        // Check if this is a major grid line
        bool isMajor = (std::abs(std::fmod(y, ySpacing * 5.0f)) < ySpacing * 0.01f) ||
        (std::abs(y) < ySpacing * 0.01f); // Always make zero line major

        if (isMajor) {
            glColor3f(0.3f, 0.3f, 0.35f);
            glLineWidth(1.5f);
        } else {
            glColor3f(0.2f, 0.2f, 0.25f);
            glLineWidth(1.0f);
        }

        drawLine(-1.0f, yNorm, 1.0f, yNorm, aspect);
    }
}

// Function to draw grid labels
void drawGridLabels(float aspect, const PlotConfig& config) {
    // Note: This is a simplified version.  For proper text rendering,
    // you'd want to use FreeType or a similar library
    // This version just shows where labels would go

    float xRange = config.xMax - config.xMin;
    float yRange = config.yMax - config.yMin;

    float xSpacing = calculateNiceGridSpacing(xRange, 10);
    float ySpacing = calculateNiceGridSpacing(yRange, 8);

    // You could add text rendering here using a library like FreeType
    // For now, we'll just mark the grid with lines
}

// Function to plot time series data
void plotTimeSeries(const TimeSeries& series, float aspect, const PlotConfig& config) {
    if (series.timeData.size() < 2) return;

    glColor3f(series.r, series.g, series.b);
    glLineWidth(series. lineWidth);

    glBegin(GL_LINE_STRIP);
    for (size_t i = 0; i < series.timeData.size(); ++i) {
        float t = series.timeData[i];
        float v = series.valueData[i];

        // Only plot if within visible range
        if (t < config.xMin || t > config.xMax) continue;

        // Map to normalized coordinates [-1, 1]
        float xNorm = 2.0f * (t - config.xMin) / (config.xMax - config.xMin) - 1.0f;
        float yNorm = 2.0f * (v - config.yMin) / (config.yMax - config.yMin) - 1.0f;

        // Clamp to visible range
        yNorm = std::max(-1.0f, std::min(1.0f, yNorm));

        // Apply aspect ratio correction
        float xScreen, yScreen;
        normalizedToScreen(xNorm, yNorm, aspect, xScreen, yScreen);

        glVertex2f(xScreen, yScreen);
    }
    glEnd();
}

// Function to update plot configuration based on moving window
void updatePlotConfig(PlotConfig& config, const std::vector<TimeSeries*>& allSeries, float currentTime) {
    // Update time window to follow current time
    config.xMax = currentTime;
    config. xMin = currentTime - config.windowWidth;

    if (config.autoScale && ! allSeries.empty()) {
        // Auto-scale Y axis based on visible data
        bool firstPoint = true;
        float minVal = 0, maxVal = 0;

        for (const auto* series : allSeries) {
            for (size_t i = 0; i < series->timeData.size(); ++i) {
                float t = series->timeData[i];
                if (t >= config.xMin && t <= config.xMax) {
                    float v = series->valueData[i];
                    if (firstPoint) {
                        minVal = maxVal = v;
                        firstPoint = false;
                    } else {
                        minVal = std::min(minVal, v);
                        maxVal = std::max(maxVal, v);
                    }
                }
            }
        }

        if (! firstPoint) {
            // Add 10% padding
            float range = maxVal - minVal;
            if (range < 0.001f) range = 1.0f;  // Avoid division by zero
            config.yMin = minVal - range * 0.1f;
            config.yMax = maxVal + range * 0.1f;
        }
    }
}

// Callback for window resize
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    int windowWidth = 1200;
    int windowHeight = 600;
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight,
                                          "2D Time-Varying Plot - Moving Grid", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Enable VSync
    glfwSwapInterval(1);

    // Enable antialiasing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // Create time series for multiple signals
    TimeSeries signal1(0.2f, 1.0f, 0.5f, 2.5f);  // Green-ish
    TimeSeries signal2(1.0f, 0.4f, 0.0f, 2.0f);  // Orange
    TimeSeries signal3(0.4f, 0.6f, 1.0f, 2.0f);  // Blue

    std::vector<TimeSeries*> allSeries = {&signal1, &signal2, &signal3};

    // Plot configuration
    PlotConfig config;
    config.windowWidth = 10.0f;  // Show last 10 seconds
    config.autoScale = true;

    // Time tracking
    double lastTime = glfwGetTime();
    float currentTime = 0.0f;
    float frequency1 = 1.0f;   // 1 Hz
    float frequency2 = 2.5f;   // 2.5 Hz
    float frequency3 = 0.5f;   // 0.5 Hz

    std::cout << "Controls:" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "  SPACE - Pause/Resume" << std::endl;
    std:: cout << "  R - Reset" << std::endl;
    std::cout << "  +/- - Adjust time window" << std::endl;
    std:: cout << "  A - Toggle auto-scaling" << std::endl;
    std::cout << "\nThe grid now moves with the plot!" << std::endl;

    bool paused = false;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        double now = glfwGetTime();
        float deltaTime = static_cast<float>(now - lastTime);
        lastTime = now;

        // Handle keyboard input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            static double lastPauseTime = 0;
            if (now - lastPauseTime > 0.3) {  // Debounce
                paused = ! paused;
                lastPauseTime = now;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            currentTime = 0.0f;
            for (auto* series : allSeries) {
                series->clear();
            }
        }
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
            config.windowWidth = std::max(1.0f, config.windowWidth - 0.1f);
            }
            if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
                config.windowWidth = std::min(60.0f, config.windowWidth + 0.1f);
                }
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                    static double lastToggleTime = 0;
                    if (now - lastToggleTime > 0.3) {
                        config.autoScale = !config.autoScale;
                        if (! config.autoScale) {
                            // Set fixed scale when turning off auto-scale
                            config.yMin = -2.0f;
                            config.yMax = 2.0f;
                        }
                        lastToggleTime = now;
                        std::cout << "Auto-scaling:  " << (config.autoScale ?  "ON" : "OFF") << std::endl;
                    }
                }

                // Update simulation
                if (! paused) {
                    currentTime += deltaTime;

                    // Generate new data points for each signal
                    // Signal 1: Sine wave
                    float value1 = std::sin(2.0f * M_PI * frequency1 * currentTime);

                    // Signal 2: Cosine wave with amplitude modulation
                    float value2 = std::cos(2.0f * M_PI * frequency2 * currentTime) *
                    (0.5f + 0.5f * std::sin(0.5f * currentTime));

                    // Signal 3: Sum of two sine waves (beat pattern)
                    float value3 = 0.5f * (std::sin(2.0f * M_PI * frequency3 * currentTime) +
                    std::sin(2.0f * M_PI * (frequency3 + 0.1f) * currentTime));

                    signal1.addPoint(currentTime, value1);
                    signal2.addPoint(currentTime, value2);
                    signal3.addPoint(currentTime, value3);
                }

                // Update plot configuration (this updates xMin and xMax based on current time)
                updatePlotConfig(config, allSeries, currentTime);

                // Get window size and calculate aspect ratio
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);
                float aspect = (height > 0) ? (float)width / (float)height : 1.0f;

                // Set viewport
                glViewport(0, 0, width, height);

                // Clear the screen
                glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                // Draw moving grid (now it moves with the data!)
                drawGrid(aspect, config);

                // Draw axes
                drawAxes(aspect, config);

                // Plot all time series
                for (const auto* series : allSeries) {
                    plotTimeSeries(*series, aspect, config);
                }

                // Draw current time indicator (vertical line at right edge)
                if (! paused) {
                    glColor3f(1.0f, 1.0f, 0.0f);
                    glLineWidth(2.0f);
                    drawLine(1.0f, -1.0f, 1.0f, 1.0f, aspect);
                }

                // Display time info in window title
                std::ostringstream title;
                title << "2D Time-Varying Plot - Moving Grid | Time: "
                << std::fixed << std::setprecision(2) << currentTime
                << "s | Window: " << config.windowWidth << "s"
                << (paused ? " [PAUSED]" : "");
                glfwSetWindowTitle(window, title.str().c_str());

                // Swap front and back buffers
                glfwSwapBuffers(window);

                // Poll for and process events
                glfwPollEvents();
    }

    // Clean up
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
