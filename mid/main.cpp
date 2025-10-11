#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <string>

using namespace glm;

// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera
vec3 cameraPos = vec3(0.0f, 3.0f, 8.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);

// Character
vec3 characterPos = vec3(0.0f, 0.0f, 0.0f);
float characterRotation = 0.0f;

// Animation
float animationTime = 0.0f;
bool isMoving = false;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Bone structure
struct Bone {
    std::string name;
    int parentIndex;
    mat4 localTransform;
    mat4 globalTransform;
    vec3 basePosition;
    vec3 color;
    vec3 size; // width, height, depth
};

class BipedSkeleton {
public:
    std::vector<Bone> bones;

    BipedSkeleton() {
        setupSkeleton();
    }

    void setupSkeleton() {
        bones.clear();

        // Bone 0: Hips/Pelvis (root) - НЕВИДИМЫЙ, только для трансформаций
        Bone hips;
        hips.name = "Hips";
        hips.parentIndex = -1;
        hips.localTransform = mat4(1.0f);
        hips.globalTransform = mat4(1.0f);
        hips.basePosition = vec3(0.0f, 0.0f, 0.0f);
        hips.color = vec3(0.0f, 0.0f, 0.0f); // ЧЕРНЫЙ (невидимый)
        hips.size = vec3(0.0f, 0.0f, 0.0f); // НУЛЕВОЙ размер
        bones.push_back(hips);

        // Bone 1: Spine/Torso
        Bone spine;
        spine.name = "Spine";
        spine.parentIndex = 0;
        spine.localTransform = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)); // Начинается от бедер
        spine.globalTransform = mat4(1.0f);
        spine.basePosition = vec3(0.0f, 0.0f, 0.0f);
        spine.color = vec3(0.0f, 1.0f, 0.0f); // Зеленый
        spine.size = vec3(0.5f, 0.8f, 0.25f); // ШИРОКОЕ тело
        bones.push_back(spine);

        // Bone 2: Head 
        Bone head;
        head.name = "Head";
        head.parentIndex = 1;
        head.localTransform = translate(mat4(1.0f), vec3(0.0f, 0.7f, 0.0f)); // ОПУЩЕНА с 0.8f до 0.7f
        head.globalTransform = mat4(1.0f);
        head.basePosition = vec3(0.0f, 0.7f, 0.0f); // ОПУЩЕНА с 0.8f до 0.7f
        head.color = vec3(1.0f, 1.0f, 0.0f); // Желтый
        head.size = vec3(0.4f, 0.4f, 0.4f); // БОЛЬШАЯ голова
        bones.push_back(head);

        // Bone 3: Left Arm 
        Bone leftArm;
        leftArm.name = "LeftArm";
        leftArm.parentIndex = 1;
        leftArm.localTransform = translate(mat4(1.0f), vec3(-0.4f, 0.2f, 0.0f)); // ОПУЩЕНА с 0.4f до 0.2f
        leftArm.globalTransform = mat4(1.0f);
        leftArm.basePosition = vec3(-0.4f, 0.2f, 0.0f); // ОПУЩЕНА с 0.4f до 0.2f
        leftArm.color = vec3(1.0f, 0.0f, 0.0f); // Красный
        leftArm.size = vec3(0.2f, 0.5f, 0.2f); // ТОЛСТАЯ рука
        bones.push_back(leftArm);

        // Bone 4: Right Arm 
        Bone rightArm;
        rightArm.name = "RightArm";
        rightArm.parentIndex = 1;
        rightArm.localTransform = translate(mat4(1.0f), vec3(0.4f, 0.2f, 0.0f)); // ОПУЩЕНА с 0.4f до 0.2f
        rightArm.globalTransform = mat4(1.0f);
        rightArm.basePosition = vec3(0.4f, 0.2f, 0.0f); // ОПУЩЕНА с 0.4f до 0.2f
        rightArm.color = vec3(1.0f, 0.0f, 0.0f); // Красный
        rightArm.size = vec3(0.2f, 0.5f, 0.2f); // ТОЛСТАЯ рука
        bones.push_back(rightArm);

        // Bone 5: Left Leg 
        Bone leftLeg;
        leftLeg.name = "LeftLeg";
        leftLeg.parentIndex = 0;
        leftLeg.localTransform = translate(mat4(1.0f), vec3(-0.15f, -0.8f, 0.0f));
        leftLeg.globalTransform = mat4(1.0f);
        leftLeg.basePosition = vec3(-0.15f, -0.8f, 0.0f);
        leftLeg.color = vec3(0.5f, 0.0f, 0.5f); // Фиолетовый
        leftLeg.size = vec3(0.2f, 0.8f, 0.2f); // ТОЛСТАЯ нога
        bones.push_back(leftLeg);

        // Bone 6: Right Leg 
        Bone rightLeg;
        rightLeg.name = "RightLeg";
        rightLeg.parentIndex = 0;
        rightLeg.localTransform = translate(mat4(1.0f), vec3(0.15f, -0.8f, 0.0f));
        rightLeg.globalTransform = mat4(1.0f);
        rightLeg.basePosition = vec3(0.15f, -0.8f, 0.0f);
        rightLeg.color = vec3(0.5f, 0.0f, 0.5f); // Фиолетовый
        rightLeg.size = vec3(0.2f, 0.8f, 0.2f); // ТОЛСТАЯ нога
        bones.push_back(rightLeg);

        updateGlobalTransforms();
    }

    void updateGlobalTransforms() {
        for (size_t i = 0; i < bones.size(); i++) {
            if (bones[i].parentIndex == -1) {
                bones[i].globalTransform = bones[i].localTransform;
            }
            else {
                if (bones[i].parentIndex >= 0 && bones[i].parentIndex < (int)bones.size()) {
                    bones[i].globalTransform = bones[bones[i].parentIndex].globalTransform * bones[i].localTransform;
                }
                else {
                    bones[i].globalTransform = bones[i].localTransform;
                }
            }
        }
    }

    void update(float time, bool moving) {
        // Reset to base transforms
        setupSkeleton();

        if (moving) {
            // Walking animation - более выраженная для толстого персонажа
            float walkCycle = sin(time * 8.0f);
            float armSwing = -walkCycle;

            // Ноги двигаются сильнее
            bones[5].localTransform = translate(mat4(1.0f), bones[5].basePosition) *
                rotate(mat4(1.0f), walkCycle * 1.0f, vec3(1.0f, 0.0f, 0.0f));
            bones[6].localTransform = translate(mat4(1.0f), bones[6].basePosition) *
                rotate(mat4(1.0f), -walkCycle * 1.0f, vec3(1.0f, 0.0f, 0.0f));

            // Руки качаются сильно (теперь от правильной позиции плеч)
            bones[3].localTransform = translate(mat4(1.0f), bones[3].basePosition) *
                rotate(mat4(1.0f), armSwing * 0.8f, vec3(1.0f, 0.0f, 0.0f));
            bones[4].localTransform = translate(mat4(1.0f), bones[4].basePosition) *
                rotate(mat4(1.0f), -armSwing * 0.8f, vec3(1.0f, 0.0f, 0.0f));

            // Тело подпрыгивает
            float bob = abs(sin(time * 8.0f)) * 0.08f;
            bones[0].localTransform = translate(mat4(1.0f), vec3(0.0f, bob, 0.0f));
        }

        updateGlobalTransforms();
    }
};

BipedSkeleton characterSkeleton;

// Simple shaders
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec3 ourColor;

void main()
{
    // Пропускаем черные кости (невидимые)
    if (ourColor == vec3(0.0, 0.0, 0.0)) {
        discard;
    }
    FragColor = vec4(ourColor, 1.0);
}
)";

// Function prototypes
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
unsigned int compileShader(unsigned int type, const char* source);
unsigned int createShaderProgram();
void createCubeMesh(std::vector<float>& vertices, std::vector<unsigned int>& indices, const vec3& size, const vec3& color);

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Minecraft-style Biped - FIXED ARMS", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Controls: W-forward, S-backward, A-rotate left, D-rotate right" << std::endl;

    // Create shader program
    unsigned int shaderProgram = createShaderProgram();
    if (shaderProgram == 0) {
        std::cerr << "Failed to create shader program" << std::endl;
        return -1;
    }

    // Create mesh for each bone with proper sizes and colors
    std::vector<std::vector<float>> boneVertices(7);
    std::vector<std::vector<unsigned int>> boneIndices(7);
    std::vector<unsigned int> VAOs(7), VBOs(7), EBOs(7);

    for (int i = 0; i < 7; i++) {
        createCubeMesh(boneVertices[i], boneIndices[i], characterSkeleton.bones[i].size, characterSkeleton.bones[i].color);

        glGenVertexArrays(1, &VAOs[i]);
        glGenBuffers(1, &VBOs[i]);
        glGenBuffers(1, &EBOs[i]);

        glBindVertexArray(VAOs[i]);

        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, boneVertices[i].size() * sizeof(float), boneVertices[i].data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, boneIndices[i].size() * sizeof(unsigned int), boneIndices[i].data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // Color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    // Create ground - ОПУЩЕНА ВНИЗ чтобы персонаж стоял на ней
    std::vector<float> groundVertices = {
        -20.0f, -1.0f, -20.0f, 0.2f, 0.6f, 0.2f,  // ЗЕМЛЯ НИЖЕ
         20.0f, -1.0f, -20.0f, 0.2f, 0.6f, 0.2f,
         20.0f, -1.0f,  20.0f, 0.2f, 0.6f, 0.2f,
        -20.0f, -1.0f,  20.0f, 0.2f, 0.6f, 0.2f,
    };
    std::vector<unsigned int> groundIndices = { 0, 1, 2, 2, 3, 0 };

    unsigned int groundVAO, groundVBO, groundEBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);
    glGenBuffers(1, &groundEBO);

    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, groundVertices.size() * sizeof(float), groundVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, groundIndices.size() * sizeof(unsigned int), groundIndices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    std::cout << "Starting main loop..." << std::endl;

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        processInput(window);

        // Update animation
        if (isMoving) {
            animationTime += deltaTime;
        }
        characterSkeleton.update(animationTime, isMoving);

        // Render
        glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // Sky blue
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader
        glUseProgram(shaderProgram);

        // Set up matrices
        mat4 projection = perspective(radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        mat4 view = lookAt(cameraPos, characterPos + vec3(0.0f, 1.0f, 0.0f), cameraUp);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, value_ptr(view));

        // Draw ground
        mat4 groundModel = mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, value_ptr(groundModel));
        glBindVertexArray(groundVAO);
        glDrawElements(GL_TRIANGLES, groundIndices.size(), GL_UNSIGNED_INT, 0);

        // Draw each bone (пропускаем кость 0 - бедра)
        for (int i = 1; i < characterSkeleton.bones.size(); i++) { // Начинаем с 1!
            mat4 model = mat4(1.0f);
            model = translate(model, characterPos);
            model = rotate(model, characterRotation, vec3(0.0f, 1.0f, 0.0f));
            model = model * characterSkeleton.bones[i].globalTransform;

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, value_ptr(model));
            glBindVertexArray(VAOs[i]);
            glDrawElements(GL_TRIANGLES, boneIndices[i].size(), GL_UNSIGNED_INT, 0);
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    for (int i = 0; i < 7; i++) {
        glDeleteVertexArrays(1, &VAOs[i]);
        glDeleteBuffers(1, &VBOs[i]);
        glDeleteBuffers(1, &EBOs[i]);
    }
    glDeleteVertexArrays(1, &groundVAO);
    glDeleteBuffers(1, &groundVBO);
    glDeleteBuffers(1, &groundEBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float moveSpeed = 3.0f * deltaTime;
    float rotationSpeed = 2.0f * deltaTime;
    isMoving = false;

    // ИСПРАВЛЕНО: W - вперед, S - назад
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        characterPos += vec3(sin(characterRotation), 0.0f, cos(characterRotation)) * moveSpeed;
        isMoving = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        characterPos -= vec3(sin(characterRotation), 0.0f, cos(characterRotation)) * moveSpeed;
        isMoving = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        characterRotation += rotationSpeed;
        isMoving = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        characterRotation -= rotationSpeed;
        isMoving = true;
    }

    // Update camera to follow character
    cameraPos = characterPos + vec3(-sin(characterRotation) * 6.0f, 3.0f, -cos(characterRotation) * 6.0f);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        return 0;
    }

    return shader;
}

unsigned int createShaderProgram() {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void createCubeMesh(std::vector<float>& vertices, std::vector<unsigned int>& indices, const vec3& size, const vec3& color) {
    vec3 halfSize = size * 0.5f;

    float cubeVertices[] = {
        // Positions          // Colors
        // Front face
        -halfSize.x, -halfSize.y,  halfSize.z, color.r, color.g, color.b,
         halfSize.x, -halfSize.y,  halfSize.z, color.r, color.g, color.b,
         halfSize.x,  halfSize.y,  halfSize.z, color.r, color.g, color.b,
        -halfSize.x,  halfSize.y,  halfSize.z, color.r, color.g, color.b,

        // Back face
        -halfSize.x, -halfSize.y, -halfSize.z, color.r * 0.8f, color.g * 0.8f, color.b * 0.8f,
         halfSize.x, -halfSize.y, -halfSize.z, color.r * 0.8f, color.g * 0.8f, color.b * 0.8f,
         halfSize.x,  halfSize.y, -halfSize.z, color.r * 0.8f, color.g * 0.8f, color.b * 0.8f,
        -halfSize.x,  halfSize.y, -halfSize.z, color.r * 0.8f, color.g * 0.8f, color.b * 0.8f,

        // Left face
        -halfSize.x,  halfSize.y,  halfSize.z, color.r * 0.9f, color.g * 0.9f, color.b * 0.9f,
        -halfSize.x,  halfSize.y, -halfSize.z, color.r * 0.9f, color.g * 0.9f, color.b * 0.9f,
        -halfSize.x, -halfSize.y, -halfSize.z, color.r * 0.9f, color.g * 0.9f, color.b * 0.9f,
        -halfSize.x, -halfSize.y,  halfSize.z, color.r * 0.9f, color.g * 0.9f, color.b * 0.9f,

        // Right face
         halfSize.x,  halfSize.y,  halfSize.z, color.r * 0.7f, color.g * 0.7f, color.b * 0.7f,
         halfSize.x,  halfSize.y, -halfSize.z, color.r * 0.7f, color.g * 0.7f, color.b * 0.7f,
         halfSize.x, -halfSize.y, -halfSize.z, color.r * 0.7f, color.g * 0.7f, color.b * 0.7f,
         halfSize.x, -halfSize.y,  halfSize.z, color.r * 0.7f, color.g * 0.7f, color.b * 0.7f,

         // Top face
         -halfSize.x,  halfSize.y, -halfSize.z, color.r, color.g, color.b,
          halfSize.x,  halfSize.y, -halfSize.z, color.r, color.g, color.b,
          halfSize.x,  halfSize.y,  halfSize.z, color.r, color.g, color.b,
         -halfSize.x,  halfSize.y,  halfSize.z, color.r, color.g, color.b,

         // Bottom face
         -halfSize.x, -halfSize.y, -halfSize.z, color.r * 0.5f, color.g * 0.5f, color.b * 0.5f,
          halfSize.x, -halfSize.y, -halfSize.z, color.r * 0.5f, color.g * 0.5f, color.b * 0.5f,
          halfSize.x, -halfSize.y,  halfSize.z, color.r * 0.5f, color.g * 0.5f, color.b * 0.5f,
         -halfSize.x, -halfSize.y,  halfSize.z, color.r * 0.5f, color.g * 0.5f, color.b * 0.5f,
    };

    unsigned int cubeIndices[] = {
        // Front
        0, 1, 2, 2, 3, 0,
        // Back
        4, 5, 6, 6, 7, 4,
        // Left
        8, 9, 10, 10, 11, 8,
        // Right
        12, 13, 14, 14, 15, 12,
        // Top
        16, 17, 18, 18, 19, 16,
        // Bottom
        20, 21, 22, 22, 23, 20
    };

    vertices.assign(cubeVertices, cubeVertices + sizeof(cubeVertices) / sizeof(float));
    indices.assign(cubeIndices, cubeIndices + sizeof(cubeIndices) / sizeof(unsigned int));
}