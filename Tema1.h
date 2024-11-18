#pragma once

#include "components/simple_scene.h"
#include <vector>

namespace m1
{
    class Tema1 : public gfxc::SimpleScene
    {
    public:
        Tema1();
        ~Tema1();

        void Init() override;

        struct Projectile {
            glm::vec2 position;
            glm::vec2 velocity;
            int tankId;
            bool active;

            Projectile(const glm::vec2& pos, const glm::vec2& vel, int tankId)
                : position(pos), velocity(vel), tankId(tankId), active(true) {}
        };

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;

        void GenerateTerrainMesh();
        void UpdateTank();
		void UpdateTank2();
        void SlideTerrain();
		void DrawTank();
		void DrawTank2();
		void DrawHealthBars();
        void CreateTankMeshes();
		void LaunchProjectile(int tankId);
        void DeformTerrain(const glm::vec2& impactPoint);
		void UpdateProjectiles(float deltaTimeSeconds);
        bool CheckCollisionWithTerrain(const glm::vec2& position);
		void DrawProjectiles();
		void DrawTrajectoryPreview();
        bool CheckCollisionWithTank(const glm::vec2& position, int tankId);

    protected:
        std::vector<float> heightMap;
        glm::mat3 modelMatrix;
		glm::vec2 barrelTipPosition1, barrelTipPosition2;
		float tankX, tankY, tankRotation, interval;
		float tankX2, tankY2, tankRotation2, interval2;
		int tankIndex, tankIndex2;
		float gravity, projectileSpeed, projectileRadius, collisionError;
        float radius, slideThreshold, slideAmount;
        float lastDeltaTime, turretRotation, turretRotation2;
		float tankHealth1, tankHealth2;
		float activeTank1, activeTank2;
        std::vector<Projectile> projectiles;
    };
}   // namespace m1
