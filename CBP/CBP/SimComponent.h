#pragma once

namespace CBP
{
    //class SimObject;

#ifdef _CBP_ENABLE_DEBUG
    struct SimDebugInfo
    {
        NiTransform worldTransform;
        NiTransform localTransform;

        NiTransform worldTransformParent;
        NiTransform localTransformParent;

        std::string parentNodeName;
    };
#endif

    class SimComponent
    {
        struct Force
        {
            uint32_t steps;
            NiPoint3 force;
        };

        class Collider
        {
            static constexpr float crdrmul = std::numbers::pi_v<float> / 180.0f;

        public:
            Collider(SimComponent& a_parent);

            Collider() = delete;
            Collider(const Collider& a_rhs) = delete;
            Collider(Collider&& a_rhs) = delete;

            bool Create(ColliderShape a_shape);
            bool Destroy();
            inline void Update();
            void Reset();

            inline void SetColliderRotation(float a_x, float a_y, float a_z)
            {
                m_colRot = r3d::Quaternion::fromEulerAngles(
                    a_x * crdrmul,
                    a_y * crdrmul,
                    a_z * crdrmul
                );
            }

            inline void SetRadius(float a_val) {
                m_radius = a_val;
                UpdateRadius();
            }

            inline void UpdateRadius()
            {
                if (!m_created)
                    return;

                auto rad = m_radius * m_nodeScale;
                if (rad > 0.0f) {
                    if (m_shape == ColliderShape::Capsule)
                        m_capsuleShape->setRadius(rad);
                    else if (m_shape == ColliderShape::Sphere)
                        m_sphereShape->setRadius(rad);
                }
            }

            inline void UpdateHeight()
            {
                if (!m_created)
                    return;

                if (m_shape == ColliderShape::Capsule) {
                    auto height = std::max(m_height * m_nodeScale, 0.001f);
                    m_capsuleShape->setHeight(height);
                }
            }

            inline void SetExtent(const r3d::Vector3& a_extent)
            {
                m_extent = a_extent;
                UpdateExtent();
            }

            inline void SetExtent(r3d::Vector3&& a_extent)
            {
                m_extent = std::move(a_extent);
                UpdateExtent();
            }

            inline void UpdateExtent()
            {
                if (!m_created)
                    return;

                if (m_shape == ColliderShape::Box)
                    m_boxShape->setHalfExtents(m_extent * m_nodeScale);
            }

            inline void SetHeight(float a_val) {
                m_height = a_val;
                UpdateHeight();
            }

            inline void SetOffset(const NiPoint3& a_offset) {
                m_bodyOffset = a_offset;
            }

            inline void SetOffset(float a_x, float a_y, float a_z) {
                m_bodyOffset.x = a_x;
                m_bodyOffset.y = a_y;
                m_bodyOffset.z = a_z;
            }

            [[nodiscard]] inline bool IsActive() const {
                return m_created && m_active;
            }

            [[nodiscard]] inline const auto& GetSphereOffset() const {
                return m_bodyOffset;
            }

            [[nodiscard]] inline auto GetColliderShape() const {
                return m_shape;
            }

            inline void SetShouldProcess(bool a_switch) {
                m_process = a_switch;
                if (IsActive())
                    m_body->setIsActive(a_switch);
            }

        private:
            r3d::CollisionBody* m_body;
            r3d::Collider* m_collider;

            union
            {
                r3d::SphereShape* m_sphereShape;
                r3d::CapsuleShape* m_capsuleShape;
                r3d::BoxShape* m_boxShape;
            };

            r3d::Vector3 m_extent;
            NiPoint3 m_bodyOffset;
            r3d::Quaternion m_colRot;
            r3d::Transform m_transform;

            ColliderShape m_shape;

            float m_nodeScale;
            float m_radius;
            float m_height;

            bool m_created;
            bool m_active;
            bool m_process;

            SimComponent& m_parent;
        };

    private:
        bool UpdateWeightData(
            Actor* a_actor,
            const configComponent_t& a_config,
            const configNode_t& a_nodeConf);

        NiPoint3 m_npCogOffset;
        NiPoint3 m_npGravityCorrection;

        NiPoint3 m_oldWorldPos;
        NiPoint3 m_velocity;

        NiTransform m_initialTransform;
        bool m_hasScaleOverride;

        Collider m_collisionData;

        std::queue<Force> m_applyForceQueue;

        std::string m_configGroupName;

        configComponent_t m_conf;

        bool m_collisions;
        bool m_movement;

        float m_colRad = 1.0f;
        float m_colHeight = 0.001f;
        float m_colCapsuleHeight = 0.0f;
        float m_colOffsetX = 0.0f;
        float m_colOffsetY = 0.0f;
        float m_colOffsetZ = 0.0f;
        float m_resistance = 0.0f;
        r3d::Vector3 m_extent;

        uint64_t m_groupId;
        uint64_t m_parentId;

        bool m_resistanceOn;
        bool m_rotScaleOn;

        NiPointer<NiAVObject> m_obj;
        NiPointer<NiAVObject> m_objParent;

        NiAVObject::ControllerUpdateContext m_updateCtx;

#ifdef _CBP_ENABLE_DEBUG
        SimDebugInfo m_debugInfo;
#endif

        Game::FormID m_formid;

        __forceinline void ClampVelocity()
        {
            float len = m_velocity.Length();
            if (len <= 1000.0f)
                return;

            m_velocity.x /= len;
            m_velocity.y /= len;
            m_velocity.z /= len;
            m_velocity *= 1000.0f;
        }
    public:
        SimComponent(
            Actor* a_actor,
            NiAVObject* m_obj,
            const std::string& a_configBoneName,
            const configComponent_t& config,
            uint64_t a_parentId,
            uint64_t a_groupId,
            bool a_collisions,
            bool a_movement,
            const configNode_t& a_nodeConf
        );

        SimComponent() = delete;
        SimComponent(const SimComponent& a_rhs) = delete;
        SimComponent(SimComponent&& a_rhs) = delete;

        void Release();

        void UpdateConfig(
            Actor* a_actor,
            const configComponent_t& a_physConf,
            bool a_collisions,
            bool a_movement,
            const configNode_t& a_nodeConf) noexcept;

        void UpdateMovement(float timeStep);
        void UpdateVelocity();
        void Reset();

        void ApplyForce(uint32_t a_steps, const NiPoint3& a_force);

#ifdef _CBP_ENABLE_DEBUG
        void UpdateDebugInfo();
#endif

        __forceinline void SetVelocity(const r3d::Vector3& a_vel) {
            m_velocity.x = a_vel.x;
            m_velocity.y = a_vel.y;
            m_velocity.z = a_vel.z;
            ClampVelocity();
        }

        __forceinline void SetVelocity(const NiPoint3& a_vel)
        {
            m_velocity = a_vel;
            ClampVelocity();
        }

        __forceinline void SetVelocity(NiPoint3&& a_vel)
        {
            m_velocity = std::move(a_vel);
            ClampVelocity();
        }

        __forceinline void AddVelocity(const NiPoint3& a_vel) {
            m_velocity += a_vel;
            //ClampVelocity();
        }

        [[nodiscard]] inline const auto& GetVelocity() const {
            return m_velocity;
        }

        [[nodiscard]] inline const auto& GetConfig() const {
            return m_conf;
        }

        [[nodiscard]] inline const auto& GetConfigGroupName() const {
            return m_configGroupName;
        }

        [[nodiscard]] inline bool IsSameGroup(const SimComponent& a_rhs) const {
            return a_rhs.m_groupId != 0 && m_groupId != 0 &&
                a_rhs.m_parentId == m_parentId &&
                a_rhs.m_groupId == m_groupId;
        }

        inline void UpdateGroupInfo(uint64_t a_groupId) {
            m_groupId = a_groupId;
        };

        [[nodiscard]] inline bool HasMovement() const {
            return m_movement;
        }

        [[nodiscard]] inline bool HasActiveCollider() const {
            return m_collisionData.IsActive();
        }

        [[nodiscard]] inline bool HasCollision() const {
            return m_collisions;
        }

        [[nodiscard]] inline const auto& GetPos() const {
            return m_obj->m_worldTransform.pos;
        }

        [[nodiscard]] inline auto GetCenterOfMass() const {
            return m_obj->m_worldTransform * m_npCogOffset;
        }

        [[nodiscard]] inline float GetNodeScale() const {
            return m_obj->m_worldTransform.scale;
        }

        [[nodiscard]] inline auto& GetCollider() {
            return m_collisionData;
        }

        [[nodiscard]] inline const auto& GetCollider() const {
            return m_collisionData;
        }

#ifdef _CBP_ENABLE_DEBUG
        [[nodiscard]] inline const auto& GetDebugInfo() const {
            return m_debugInfo;
        }
#endif
    };
}