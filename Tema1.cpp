#include "lab_m1/Tema1/Tema1.h"
#include <vector>
#include <iostream>
#include <math.h>
#include "lab_m1/Tema1/transform2D.h"
#include <glm/vec2.hpp>
#include "lab_m1/Tema1/object2D.h"

using namespace std;
using namespace m1;

Tema1::Tema1() {
    gravity = 100.0f;
    projectileSpeed = 300.0f;
    projectileRadius = 5.0f;
    collisionError = 0.0f;
    radius = 50.0f;
    slideThreshold = 80.0f;
    slideAmount = 0.50f;

    turretRotation = 0.0f;
    turretRotation2 = glm::pi<float>();

    tankHealth1 = 7;
    tankHealth2 = 7;
}

Tema1::~Tema1() {
}

void Tema1::LaunchProjectile(int tankId) {

    float totalRotation;
    glm::vec2 turretTip, initialVelocity;

    if (tankId == 1 && activeTank1) {
        totalRotation = turretRotation + tankRotation;

        turretTip = glm::vec2(
            barrelTipPosition1.y + 5 * cos(totalRotation),
            barrelTipPosition1.x + 5 * sin(totalRotation)
        );
        initialVelocity = glm::vec2(
            projectileSpeed * sin(totalRotation),
            projectileSpeed * cos(totalRotation)
        );
    }
    else if (tankId == 2 && activeTank2) {
        totalRotation = turretRotation2 + tankRotation2;
        turretTip = glm::vec2(
            barrelTipPosition2.y + 5 * cos(totalRotation),
            barrelTipPosition2.x + 5 * sin(totalRotation)
        );
        initialVelocity = glm::vec2(
            projectileSpeed * sin(totalRotation),
            projectileSpeed * cos(totalRotation)
        );
    }

    projectiles.push_back(Projectile(turretTip, initialVelocity, tankId));
}

void Tema1::DrawTrajectoryPreview() {
    if (activeTank1) {
        float totalRotation = turretRotation + tankRotation;

        glm::vec2 turretTip = glm::vec2(
            barrelTipPosition1.y + 4 * cos(totalRotation),
            barrelTipPosition1.x + 4 * sin(totalRotation)
        );

        glm::vec2 initialVelocity = glm::vec2(
            projectileSpeed * sin(totalRotation),
            projectileSpeed * cos(totalRotation)
        );

        float deltaTime = 0.05f;
        int numPoints = 50;
        glm::vec2 position = turretTip;
        glm::vec2 velocity = initialVelocity;

        for (int i = 0; i < numPoints; i++) {
            glm::vec2 nextPosition = position + velocity * deltaTime;

            velocity.x -= gravity * deltaTime * 3;

            glm::mat3 modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(position.x, position.y);
            RenderMesh2D(meshes["trajectoryPoint"], shaders["VertexColor"], modelMatrix);

            position = nextPosition;
        }
    }

    if (activeTank2) {
        float totalRotation2 = turretRotation2 + tankRotation2;

        glm::vec2 turretTip2 = glm::vec2(
            barrelTipPosition2.y + 4 * cos(totalRotation2),
            barrelTipPosition2.x + 4 * sin(totalRotation2)
        );

        glm::vec2 initialVelocity2 = glm::vec2(
            projectileSpeed * sin(totalRotation2),
            projectileSpeed * cos(totalRotation2)
        );

        float deltaTime = 0.05f;
        int numPoints = 50;
        glm::vec2 position2 = turretTip2;
        glm::vec2 velocity2 = initialVelocity2;

        for (int i = 0; i < numPoints; i++) {
            glm::vec2 nextPosition2 = position2 + velocity2 * deltaTime;

            velocity2.x -= gravity * deltaTime * 3;

            glm::mat3 modelMatrix2 = glm::mat3(1);
            modelMatrix2 *= transform2D::Translate(position2.x, position2.y);
            RenderMesh2D(meshes["trajectoryPoint"], shaders["VertexColor"], modelMatrix2);

            position2 = nextPosition2;
        }
    }
}

void Tema1::UpdateProjectiles(float deltaTime) {
    for (auto& projectile : projectiles) {
        if (projectile.active) {
            projectile.position += projectile.velocity * deltaTime;
            projectile.velocity.x -= gravity * deltaTime * 3;

            if (activeTank1 && CheckCollisionWithTank(projectile.position, 1) && projectile.tankId == 2) {
                projectile.active = false;
                tankHealth1--;
                if (tankHealth1 <= 0) {
                    cout << "Tank 1 destroyed!" << endl;
                    activeTank1 = false;
                }
            }

            else if (activeTank2 && CheckCollisionWithTank(projectile.position, 2) && projectile.tankId == 1) {
                projectile.active = false;
                tankHealth2--;
                if (tankHealth2 <= 0) {
                    cout << "Tank 2 destroyed!" << endl;
                    activeTank2 = false;
                }
            }

            else if (CheckCollisionWithTerrain(projectile.position)) {
                projectile.active = false;
                DeformTerrain(projectile.position);
                SlideTerrain();
            }
        }
    }
}


bool Tema1::CheckCollisionWithTerrain(const glm::vec2& position) {
    if (position.x < 0 || position.y >= window->GetResolution().x) {
        return false;
    }

    int segmentIndex = static_cast<int>(position.y / interval);
    if (segmentIndex < 0 || segmentIndex >= heightMap.size() - 1) {
        return false;
    }

    float Ax = segmentIndex * interval;
    float Ay = heightMap[segmentIndex];
    float Bx = (segmentIndex + 1) * interval;
    float By = heightMap[segmentIndex + 1];

    float t = (position.y - Ax) / (Bx - Ax);
    float yProjected = Ay * (1 - t) + By * t;

    return (position.x <= yProjected + collisionError);
}

void Tema1::DeformTerrain(const glm::vec2& impactPoint) {
    int impactIndex = static_cast<int>(impactPoint.y / interval);

    for (int i = impactIndex - radius / interval; i <= impactIndex + radius / interval; i++) {
        if (i >= 0 && i < heightMap.size()) {
            float distance = abs(i * interval - impactPoint.y);
            if (distance <= radius) {
                float deformation = 1.0f - (distance / radius);
                heightMap[i] -= deformation * 20.0f;
                if (heightMap[i] < 0) heightMap[i] = 0;
            }
        }
    }
    GenerateTerrainMesh();
}

void Tema1::SlideTerrain() {
    bool slide = true;

    while (slide) {
        slide = false;

        for (int i = 0; i < heightMap.size() - 1; i++) {
            float heightDifference = heightMap[i] - heightMap[i + 1];

            if (abs(heightDifference) > slideThreshold) {
                slide = true;
                float slideStep = slideAmount * lastDeltaTime;

                if (heightDifference > 0) {
                    heightMap[i] -= slideStep;
                    heightMap[i + 1] += slideStep;
                }
                else {
                    heightMap[i] += slideStep;
                    heightMap[i + 1] -= slideStep;
                }
            }
        }
    }

    GenerateTerrainMesh();
}

void Tema1::DrawProjectiles() {
    for (const auto& projectile : projectiles) {
        if (projectile.active) {
            glm::mat3 modelMatrix = glm::mat3(1);
            modelMatrix *= transform2D::Translate(projectile.position.x, projectile.position.y);
            RenderMesh2D(meshes["projectileCircle"], shaders["VertexColor"], modelMatrix);
        }
    }
}

void Tema1::CreateTankMeshes() {
    Mesh* baseLowerTrapezoid = object2D::CreateTrapezoid("baseLowerTrapezoid", glm::vec3(0.5f, 0.3f, 0.1f));
    AddMeshToList(baseLowerTrapezoid);

    Mesh* baseUpperTrapezoid = object2D::CreateTrapezoid("baseUpperTrapezoid", glm::vec3(0.6f, 0.4f, 0.2f));
    AddMeshToList(baseUpperTrapezoid);

    Mesh* turretCircle = object2D::CreateCircle("turretCircle", glm::vec3(0.7f, 0.5f, 0.3f), 20.0f, 30.0f);
    AddMeshToList(turretCircle);

    Mesh* barrelRectangle = object2D::CreateRectangle("barrelRectangle", glm::vec3(0.2f, 0.2f, 0.2f));
    AddMeshToList(barrelRectangle);

    Mesh* healthBar = object2D::CreateRectangle("healthBar", glm::vec3(0.0f, 1.0f, 0.0f));
    AddMeshToList(healthBar);
}

void Tema1::DrawTank() {
    if (activeTank1) {
        glm::mat3 modelMatrixBase = glm::mat3(1);
        modelMatrixBase *= transform2D::Translate(tankY + 15, tankX);
        modelMatrixBase *= transform2D::Rotate(tankRotation);

        glm::mat3 modelMatrixLowerBase = modelMatrixBase * transform2D::Rotate(glm::radians(180.0f)) * transform2D::Scale(1.0f, 0.5f);
        RenderMesh2D(meshes["baseLowerTrapezoid"], shaders["VertexColor"], modelMatrixLowerBase);

        glm::mat3 modelMatrixUpperBase = modelMatrixBase * transform2D::Scale(1.5f, 1.0f);
        RenderMesh2D(meshes["baseUpperTrapezoid"], shaders["VertexColor"], modelMatrixUpperBase);

        glm::mat3 modelMatrixTurret = modelMatrixBase * transform2D::Translate(15, 0);
        RenderMesh2D(meshes["turretCircle"], shaders["VertexColor"], modelMatrixTurret);

        glm::mat3 modelMatrixBarrel = modelMatrixTurret * transform2D::Translate(12, 5) * transform2D::Rotate(turretRotation);
        RenderMesh2D(meshes["barrelRectangle"], shaders["VertexColor"], modelMatrixBarrel);

        glm::vec3 barrelTip = modelMatrixBarrel * glm::vec3(0, 0, 1);
        barrelTipPosition1 = glm::vec2(barrelTip.x, barrelTip.y);
    }
}

void Tema1::DrawTank2() {
    if (activeTank2) {
        glm::mat3 modelMatrixBase = glm::mat3(1);
        modelMatrixBase *= transform2D::Translate(tankY2 + 15, tankX2);
        modelMatrixBase *= transform2D::Rotate(tankRotation2);

        glm::mat3 modelMatrixLowerBase = modelMatrixBase * transform2D::Rotate(glm::radians(180.0f)) * transform2D::Scale(1.0f, 0.5f);
        RenderMesh2D(meshes["baseLowerTrapezoid"], shaders["VertexColor"], modelMatrixLowerBase);

        glm::mat3 modelMatrixUpperBase = modelMatrixBase * transform2D::Scale(1.5f, 1.0f);
        RenderMesh2D(meshes["baseUpperTrapezoid"], shaders["VertexColor"], modelMatrixUpperBase);

        glm::mat3 modelMatrixTurret = modelMatrixBase * transform2D::Translate(15, 0);
        RenderMesh2D(meshes["turretCircle"], shaders["VertexColor"], modelMatrixTurret);

        glm::mat3 modelMatrixBarrel = modelMatrixTurret * transform2D::Translate(12, 5) * transform2D::Rotate(turretRotation2);
        RenderMesh2D(meshes["barrelRectangle"], shaders["VertexColor"], modelMatrixBarrel);

        glm::vec3 barrelTip = modelMatrixBarrel * glm::vec3(0, 0, 1);
        barrelTipPosition2 = glm::vec2(barrelTip.x, barrelTip.y);
    }
}

void Tema1::DrawHealthBars() {
    if (activeTank1) {
        float healthBarWidth = 30.0f;
        float healthBarHeight = 5.0f;
        float healthPercentage = (float)tankHealth1 / 7;
        float currentHealthBarWidth = healthBarWidth * healthPercentage;

		glm::vec2 healthBarPosition = glm::vec2(barrelTipPosition1.y + 15, barrelTipPosition1.x - 10);

        glm::mat3 modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(healthBarPosition.x, healthBarPosition.y);
        modelMatrix *= transform2D::Scale(currentHealthBarWidth / healthBarWidth, 1.0f);
        RenderMesh2D(meshes["healthBar"], shaders["VertexColor"], modelMatrix);
    }

    if (activeTank2) {
        float healthBarWidth = 30.0f;
        float healthBarHeight = 5.0f;
        float healthPercentage = (float)tankHealth2 / 7;
        float currentHealthBarWidth = healthBarWidth * healthPercentage;

		glm::vec2 healthBarPosition = glm::vec2(barrelTipPosition2.y + 15, barrelTipPosition2.x - 10);

        glm::mat3 modelMatrix = glm::mat3(1);
        modelMatrix *= transform2D::Translate(healthBarPosition.x, healthBarPosition.y);
        modelMatrix *= transform2D::Scale(currentHealthBarWidth / healthBarWidth, 1.0f);
        RenderMesh2D(meshes["healthBar"], shaders["VertexColor"], modelMatrix);
    }
}


void Tema1::UpdateTank() {
    if (tankIndex < 0 || tankIndex >= heightMap.size() - 1) return;

    float Ax = tankIndex * interval;
    float Ay = heightMap[tankIndex];
    float Bx = (tankIndex + 1) * interval;
    float By = heightMap[tankIndex + 1];

    float t = (tankX - Ax) / (Bx - Ax);
    tankY = Ay + t * (By - Ay);

    float dx = Bx - Ax;
    float dy = By - Ay;
    tankRotation = atan2(dy, dx);
}

void Tema1::UpdateTank2() {
    if (tankIndex2 < 0 || tankIndex2 >= heightMap.size() - 1) return;

    float Ax = tankIndex2 * interval;
    float Ay = heightMap[tankIndex2];
    float Bx = (tankIndex2 + 1) * interval;
    float By = heightMap[tankIndex2 + 1];

    float t = (tankX2 - Ax) / (Bx - Ax);
    tankY2 = Ay + t * (By - Ay);

    float dx = Bx - Ax;
    float dy = By - Ay;
    tankRotation2 = atan2(dy, dx);
}

bool Tema1::CheckCollisionWithTank(const glm::vec2& position, int tankId) {
    glm::vec2 tankPosition = (tankId == 1) ? glm::vec2(tankY, tankX) : glm::vec2(tankY2, tankX2);
    float distance = glm::distance(position, tankPosition);
    return distance < radius;
}

void Tema1::GenerateTerrainMesh() {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    interval = static_cast<float>(window->GetResolution().x) / (heightMap.size() - 1);
    float screenBottom = 0.0f;
    glm::ivec2 resolution = window->GetResolution();

    for (int i = 0; i < heightMap.size(); i++) {
        float x = i * interval;
        float y = heightMap[i];

        vertices.emplace_back(glm::vec3(x, y, 0), glm::vec3(0.0f, 0.8f, 0.0f));
        vertices.emplace_back(glm::vec3(x, screenBottom, 0), glm::vec3(0.0f, 0.8f, 0.0f));

        if (i < heightMap.size() - 1) {
            indices.push_back(2 * i);
            indices.push_back(2 * i + 1);
            indices.push_back(2 * (i + 1));
            indices.push_back(2 * (i + 1) + 1);
        }
    }

    Mesh* terrainMesh = new Mesh("terrain");
    terrainMesh->InitFromData(vertices, indices);
    terrainMesh->SetDrawMode(GL_TRIANGLE_STRIP);
    AddMeshToList(terrainMesh);
}

void Tema1::Init() {
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    Mesh* projectileCircle = object2D::CreateCircle("projectileCircle", glm::vec3(1.0f, 0.0f, 0.0f), projectileRadius, 30);
    AddMeshToList(projectileCircle);

    Mesh* trajectoryPoint = object2D::CreateCircle("trajectoryPoint", glm::vec3(0.2f, 0.2f, 1.0f), 3.0f, 10);
    AddMeshToList(trajectoryPoint);

    CreateTankMeshes();

    float start = 0;
    float end = static_cast<float>(resolution.x);
    int numPoints = end / 15;

    heightMap.clear();
    float A1 = 100.0f, A2 = 60.0f, A3 = 30.0f;
    float F1 = 0.01f, F2 = 0.02f, F3 = 0.015f;
    interval = (end - start) / (numPoints - 1);


    for (int i = 0; i < numPoints; ++i) {
        float t = start + i * interval;
        float y = A1 * sin(F1 * t) + A2 * sin(F2 * t) + A3 * sin(F3 * t);
        heightMap.push_back(y + resolution.y / 2);
    }

    tankIndex = numPoints / 3;
    tankX = tankIndex * interval + interval / 2.0f;

    tankIndex2 = 2 * numPoints / 3;
    tankX2 = tankIndex2 * interval + interval / 2.0f;

    GenerateTerrainMesh();
    UpdateTank();
    UpdateTank2();
}


void Tema1::Update(float deltaTimeSeconds) {
    lastDeltaTime = deltaTimeSeconds;

    OnInputUpdate(deltaTimeSeconds, 0);

    RenderMesh2D(meshes["terrain"], shaders["VertexColor"], glm::mat3(1));

    DrawTank();
    DrawTank2();

    UpdateProjectiles(deltaTimeSeconds);
    DrawProjectiles();

    DrawTrajectoryPreview();

    DrawHealthBars();
}

void Tema1::FrameStart() {
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema1::FrameEnd() {
}

void Tema1::OnInputUpdate(float deltaTime, int mods) {
    bool moved = false;

    float moveStep = 2.0f;

	if (window->KeyHold(GLFW_KEY_D) && tankX < window->GetResolution().x) {
        tankX += moveStep;
        moved = true;
    }
	if (window->KeyHold(GLFW_KEY_A) && tankX > 0) {
        tankX -= moveStep;
        moved = true;
    }

    float turretRotateStep = 1.0f * deltaTime;

    if (window->KeyHold(GLFW_KEY_W) && turretRotation < 3.14f / 2) {
        turretRotation += turretRotateStep;
    }
    if (window->KeyHold(GLFW_KEY_S) && turretRotation > 0.0f) {
        turretRotation -= turretRotateStep;
    }

    if (moved) {
        int newTankIndex = static_cast<int>(tankX / interval);
        if (newTankIndex != tankIndex) {
            tankIndex = newTankIndex;
        }
        UpdateTank();
    }

    moved = false;
	if (window->KeyHold(GLFW_KEY_RIGHT) && tankX2 < window->GetResolution().x) {
        tankX2 += moveStep;
        moved = true;
    }
    if (window->KeyHold(GLFW_KEY_LEFT) && tankX2 > 0) {
        tankX2 -= moveStep;
        moved = true;
    }

    if (window->KeyHold(GLFW_KEY_DOWN) && turretRotation2 < glm::pi<float>()) {
        turretRotation2 += turretRotateStep;
    }
    if (window->KeyHold(GLFW_KEY_UP) && turretRotation2 > glm::pi<float>() / 2) {
        turretRotation2 -= turretRotateStep;
    }

    if (moved) {
        int newTankIndex2 = static_cast<int>(tankX2 / interval);
        if (newTankIndex2 != tankIndex2) {
            tankIndex2 = newTankIndex2;
        }
        UpdateTank2();
    }
}

void Tema1::OnKeyPress(int key, int mods) {
    if (key == GLFW_KEY_SPACE) {
        LaunchProjectile(1);
    }
    if (key == GLFW_KEY_ENTER) {
        LaunchProjectile(2);
    }
}