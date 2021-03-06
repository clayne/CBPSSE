#pragma once

#include "Config.h"
#include "BoneCast.h"
#include "Common/BulletExtensions.h"

namespace CBP
{
    class SimObject;
    class SimComponent;
    class Collider;

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


    class SKMP_ALIGN_AUTO CollisionShape
    {
    public:
        SKMP_DECLARE_ALIGNED_ALLOCATOR_AUTO();

        virtual void UpdateShape() = 0;

        virtual void SetRadius(btScalar a_radius);
        virtual void SetHeight(btScalar a_height);
        virtual void SetExtent(const btVector3 & a_extent);
        virtual void SetNodeScale(btScalar a_scale);

        virtual btCollisionShape* GetBTShape() = 0;

        virtual ~CollisionShape() noexcept = default;

    protected:

        CollisionShape(btScalar a_nodeScale);

        btScalar m_nodeScale;
    };

    template <class T>
    class SKMP_ALIGN_AUTO CollisionShapeBase :
        public CollisionShape
    {

    public:

        [[nodiscard]] virtual btCollisionShape* GetBTShape();

    protected:

        virtual ~CollisionShapeBase() noexcept;

        template <typename... Args>
        CollisionShapeBase(btCollisionObject * a_collider, Args&&... a_args);

        template <typename... Args>
        SKMP_FORCEINLINE void RecreateShape(Args&&... a_args);
        SKMP_FORCEINLINE void PostUpdateShape();

        union
        {
            T* m_shape;
            btCollisionShape* m_baseShape;
        };

        btCollisionObject* m_collider;
    };

    template <class T>
    class SKMP_ALIGN_AUTO CollisionShapeTemplRH :
        public CollisionShapeBase<T>
    {
    protected:

        template <typename... Args>
        CollisionShapeTemplRH(btCollisionObject * a_collider, Args&&... a_args);

    public:

        virtual void DoRecreateShape(btScalar a_radius, btScalar a_height) = 0;

        virtual void UpdateShape();
        virtual void SetRadius(btScalar a_radius);
        virtual void SetHeight(btScalar a_height);

    protected:

        btScalar m_radius;
        btScalar m_height;

        btScalar m_currentRadius;
        btScalar m_currentHeight;
    };

    template <class T>
    class SKMP_ALIGN_AUTO CollisionShapeTemplExtent :
        public CollisionShapeBase<T>
    {
    public:

        template <typename... Args>
        CollisionShapeTemplExtent(btCollisionObject * a_collider, Args&&... a_args);

        virtual void DoRecreateShape(const btVector3 & a_extent) = 0;
        virtual void SetShapeProperties(const btVector3 & a_extent);
        virtual void UpdateShape();
        virtual void SetExtent(const btVector3 & a_extent);

    protected:
        btVector3 m_extent;
        btVector3 m_currentExtent;
    };

    class SKMP_ALIGN_AUTO CollisionShapeSphere :
        public CollisionShapeBase<btSphereShape>
    {
    public:

        CollisionShapeSphere(btCollisionObject* a_collider, btScalar a_radius);

        virtual void UpdateShape();
        virtual void SetRadius(btScalar a_radius);

    private:
        btScalar m_radius;
        btScalar m_currentRadius;
    };


    class SKMP_ALIGN_AUTO CollisionShapeCapsule :
        public CollisionShapeTemplRH<btCapsuleShape>
    {
    public:
        CollisionShapeCapsule(btCollisionObject * a_collider, btScalar a_radius, btScalar a_height);

        virtual void DoRecreateShape(btScalar a_radius, btScalar a_height);
    };

    class SKMP_ALIGN_AUTO CollisionShapeCone :
        public CollisionShapeTemplRH<btConeShape>
    {
    public:
        CollisionShapeCone(btCollisionObject * a_collider, btScalar a_radius, btScalar a_height);

        virtual void DoRecreateShape(btScalar a_radius, btScalar a_height);
    };

    class SKMP_ALIGN_AUTO CollisionShapeBox :
        public CollisionShapeTemplExtent<btBoxShape>
    {
    public:
        CollisionShapeBox(btCollisionObject * a_collider, const btVector3 & a_extent);

        virtual void DoRecreateShape(const btVector3 & a_extent);
    };

    class SKMP_ALIGN_AUTO CollisionShapeCylinder :
        public CollisionShapeTemplRH<btCylinderShape>
    {
    public:
        CollisionShapeCylinder(btCollisionObject * a_collider, btScalar a_radius, btScalar a_height);

        virtual void DoRecreateShape(btScalar a_radius, btScalar a_height);
    };

    class SKMP_ALIGN_AUTO CollisionShapeTetrahedron :
        public CollisionShapeTemplExtent<btTetrahedronShapeEx>
    {
    public:
        CollisionShapeTetrahedron(btCollisionObject * a_collider, const btVector3 & a_extent);

        virtual void DoRecreateShape(const btVector3 & a_extent);
        virtual void SetShapeProperties(const btVector3 & a_extent);

    private:
        static const btVector3 m_vertices[4];
    };

    class SKMP_ALIGN_AUTO CollisionShapeMesh :
        public CollisionShapeTemplExtent<btGImpactMeshShapePart>
    {
    public:
        CollisionShapeMesh(
            btCollisionObject * a_collider,
            btTriangleIndexVertexArray * a_data,
            const btVector3 & a_extent);

        virtual void DoRecreateShape(const btVector3 & a_extent);
        virtual void SetShapeProperties(const btVector3 & a_extent);

    private:

        btTriangleIndexVertexArray* m_triVertexArray;
    };

    class SKMP_ALIGN_AUTO CollisionShapeConvexHull :
        public CollisionShapeTemplExtent<btConvexHullShape>
    {
    public:

        CollisionShapeConvexHull(
            btCollisionObject * a_collider,
            const MeshPoint *a_data,
            int a_numVertices,
            const btVector3 & a_extent);

        virtual void DoRecreateShape(const btVector3 & a_extent);
        virtual void SetShapeProperties(const btVector3 & a_extent);

    private:

        const MeshPoint* m_convexHullPoints;
        int m_convexHullNumVertices;
    };

    class SKMP_ALIGN_AUTO Collider :
        ILog
    {
        inline static constexpr btScalar dtrmul = std::numbers::pi_v<btScalar> / 180.0f;

    public:
        Collider(SimComponent & a_parent);
        virtual ~Collider() noexcept;

        Collider() = delete;
        Collider(const Collider & a_rhs) = delete;
        Collider(Collider && a_rhs) = delete;

        bool Create(const configNode_t & a_nodeConf, ColliderShapeType a_shape);
        bool Destroy();
        SKMP_FORCEINLINE void Update();

        SKMP_FORCEINLINE void SetColliderRotation(btScalar a_x, btScalar a_y, btScalar a_z)
        {
            m_colRot.setEulerZYX(
                a_x * dtrmul,
                a_y * dtrmul,
                a_z * dtrmul
            );
        }
        
        SKMP_FORCEINLINE void SetColliderRotation(const btVector3 &a_vec)
        {
            m_colRot.setEulerZYX(
                a_vec.x() * dtrmul,
                a_vec.y() * dtrmul,
                a_vec.z() * dtrmul
            );
        }

        SKMP_FORCEINLINE void SetRadius(btScalar a_val) {
            if (m_created)
                m_colshape->SetRadius(a_val);
        }

        SKMP_FORCEINLINE void SetHeight(btScalar a_val) {
            if (m_created)
                m_colshape->SetHeight(a_val);
        }

        SKMP_FORCEINLINE void SetExtent(const btVector3 & a_extent)
        {
            if (m_created)
                m_colshape->SetExtent(a_extent);
        }

        SKMP_FORCEINLINE void SetOffset(const btVector3 & a_offset, const btVector3 & a_initial) {
            m_bodyOffset = a_offset;
            m_bodyOffsetPlusInitial = a_offset + a_initial;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool IsActive() const {
            return m_created && m_active;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool IsCreated() const {
            return m_created;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool IsBoneCast() const {
            return m_bonecast;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetSphereOffset() const {
            return m_bodyOffset;
        }

        SKMP_FORCEINLINE void SetPositionScale(btScalar a_scale) {
            m_positionScale = a_scale;
            m_doPositionScaling = a_scale != 1.0f;
        }

        SKMP_FORCEINLINE void SetRotationScale(btScalar a_scale) {
            m_doRotationScaling = a_scale != 1.0f;
            m_rotationScale = std::max(a_scale, 0.001f);
        }

        void SetShouldProcess(bool a_switch);

        SKMP_FORCEINLINE void SetOffsetParent(bool a_switch) {
            m_offsetParent = a_switch;
        }

        FN_NAMEPROC("Collider");

    private:

        void Activate();
        void Deactivate();
        void UpdateWorldData(bool a_basis);

        btMatrix3x3 m_colRot;

        btVector3 m_bodyOffset;
        btVector3 m_bodyOffsetPlusInitial;

        std::unique_ptr<btCollisionObject> m_collider;
        std::unique_ptr<CollisionShape> m_colshape;
        std::shared_ptr<const ColliderData> m_colliderData;

        ColliderShapeType m_shape;

        bool m_bonecast;
        BoneCacheUpdateID m_bcUpdateID;

        btScalar m_nodeScale;
        btScalar m_positionScale;
        btScalar m_rotationScale;

        bool m_created;
        bool m_active;
        bool m_colliderActivated;
        bool m_process;
        bool m_rotation;

        bool m_doPositionScaling;
        bool m_doRotationScaling;
        bool m_offsetParent;

        stl::fixed_string m_meshShape;

        SimComponent& m_parent;
    };

    class SKMP_ALIGN_AUTO SimComponent
    {
        struct SKMP_ALIGN_AUTO Force
        {
            SKMP_DECLARE_ALIGNED_ALLOCATOR_AUTO();

            Force(
                std::uint32_t a_steps,
                const btVector3 & a_norm)
                :
                m_numImpulses(a_steps),
                m_force(a_norm)
            {
            }

            btVector3 m_force;
            std::uint32_t m_numImpulses;
        };

        struct SKMP_ALIGN_AUTO rotationParams_t
        {
            SKMP_FORCEINLINE rotationParams_t();
            SKMP_FORCEINLINE void Zero();

            btVector3 m_axis;
            btScalar m_angle;
            //btScalar m_axisLength;
        };

        struct SKMP_ALIGN_AUTO positionData_t
        {
            btVector3 m_position;
            btMatrix3x3 m_rotation;
            
            positionData_t()
            {
                m_rotation.setIdentity();
                m_position.setValue(0, 0, 0);
            }

        };
        
        positionData_t m_wdObject;
        positionData_t m_ldObject;
        positionData_t m_wdParent;

        SimComponent* m_scParent;

        friend class Collider;

    private:

        void ColUpdateWeightData(
            Actor * a_actor,
            const configComponent_t & a_config,
            const configNode_t & a_nodeConf);

        SKMP_FORCEINLINE void ClampVelocity();

        SKMP_FORCEINLINE void ConstrainMotionBox(
            const btMatrix3x3& a_parentRot,
            const btMatrix3x3 & a_invRot,
            const btVector3 & a_target,
            btScalar a_timeStep
        );
        
        SKMP_FORCEINLINE void ConstrainMotionSphere(
            const btMatrix3x3& a_parentRot,
            const btMatrix3x3 & a_invRot,
            const btVector3 & a_target,
            btScalar a_timeStep
        );

        //SKMP_FORCEINLINE void SIMDFillObj();
        //SKMP_FORCEINLINE void SIMDFillParent();


    public:
        SKMP_DECLARE_ALIGNED_ALLOCATOR_AUTO();

        SimComponent(
            SimObject & a_parent,
            Actor * a_actor,
            NiAVObject * a_obj,
            NiNode * a_originalParentNode,
            const stl::fixed_string& a_nodeName,
            const stl::fixed_string& a_configBoneName,
            const configComponent_t & config,
            const configNode_t & a_nodeConf,
            uint64_t a_groupId,
            bool a_collisions,
            bool a_motion
        );

        virtual ~SimComponent() noexcept;

        SimComponent() = delete;
        SimComponent(const SimComponent & a_rhs) = delete;
        SimComponent(SimComponent && a_rhs) = delete;

        SimComponent& operator=(const SimComponent&) = delete;
        SimComponent& operator=(SimComponent&&) = delete;

        void UpdateConfig(
            Actor * a_actor,
            NiNode* a_parentNode,
            const configComponent_t * a_physConf,
            const configNode_t & a_nodeConf,
            bool a_collisions,
            bool a_motion) noexcept;

        void UpdateMotion(btScalar timeStep);
        SKMP_FORCEINLINE void UpdateVelocity(float a_timeStep);
        SKMP_NOINLINE void Reset();

        void ApplyForce(std::uint32_t a_steps, const btVector3 & a_force);

#ifdef _CBP_ENABLE_DEBUG
        void UpdateDebugInfo();
#endif

        SKMP_FORCEINLINE void AddVelocity(const btVector3 & a_vel) {
            m_velocity += a_vel;
        }

        SKMP_FORCEINLINE void SubVelocity(const btVector3 & a_vel) {
            m_velocity -= a_vel;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetVelocity() const {
            return m_velocity;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetConfig() const {
            return m_conf;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetConfigGroupName() const {
            return m_configGroupName;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetNodeName() const {
            return m_nodeName;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool IsSameGroup(const SimComponent & a_rhs) const
        {
            return a_rhs.m_groupId != 0 && m_groupId != 0 &&
                a_rhs.m_formid == m_formid &&
                a_rhs.m_groupId == m_groupId;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool HasMotion() const {
            return m_motion;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool HasFriction() const {
            return m_hasFriction;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool HasActiveCollider() const {
            return m_collider.IsActive();
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetPos() const {
            return m_obj->m_worldTransform.pos;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetWorldTransform() const {
            return m_obj->m_worldTransform;
        }

        [[nodiscard]] SKMP_FORCEINLINE void GetWorldTransform(Bullet::btTransformEx& a_out) const {
            a_out = m_obj->m_worldTransform;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetParentWorldTransform() const {
            return m_objParent->m_worldTransform;
        }
        
        [[nodiscard]] SKMP_FORCEINLINE void GetParentWorldTransform(Bullet::btTransformEx &a_out) const {
            a_out = m_objParent->m_worldTransform;
        }
        
        [[nodiscard]] SKMP_FORCEINLINE const auto & GetParentMatrix() const {
            return GetParentWorldData().m_rotation;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetVirtualPos() const {
            return m_virtld;
        }
        
        [[nodiscard]] SKMP_FORCEINLINE const auto &GetNodeLocalPos() const {
            return m_nodePosition;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetCenterOfGravity() const {
            return m_conf.fp.vec.cogOffset;
        }

        [[nodiscard]] SKMP_FORCEINLINE btScalar GetNodeScale() const {
            return m_obj->m_worldTransform.scale;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto& GetCollider() {
            return m_collider;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetCollider() const {
            return m_collider;
        }

        [[nodiscard]] SKMP_FORCEINLINE const btScalar GetMassInverse() const {
            return m_invMass;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto GetNode() {
            return m_obj.m_pObject;
        }
        
        [[nodiscard]] SKMP_FORCEINLINE auto GetParentNode() {
            return m_objParent.m_pObject;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto const GetNode() const {
            return m_obj.m_pObject;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool HasBoneCastCollider() const {
            return m_collider.IsCreated() && m_collider.IsBoneCast();
        }
        
        [[nodiscard]] SKMP_FORCEINLINE auto GetOriginalParentNode() const {
            return m_objParentOriginal.get();
        }
        
        SKMP_FORCEINLINE void SetSimComponentParent(SimComponent *a_parent) {
            m_scParent = a_parent;
        }
        
        [[nodiscard]] SKMP_FORCEINLINE auto GetSimComponentParent() const {
            return m_scParent;
        }

        SKMP_FORCEINLINE void ReadTransforms();
        SKMP_FORCEINLINE void WriteTransforms();

        
        /*[[nodiscard]] SKMP_FORCEINLINE bool HasBound() const {
            return m_hasBound;
        }
        
        [[nodiscard]] SKMP_FORCEINLINE const auto& GetBound() const {
            return m_bound;
        }*/

#if 0
        SKMP_FORCEINLINE void Lock() const {
            m_mutex.lock();
        }

        SKMP_FORCEINLINE void Unlock() const {
            m_mutex.unlock();
        }
#endif

#ifdef _CBP_ENABLE_DEBUG
        [[nodiscard]] SKMP_FORCEINLINE const auto& GetDebugInfo() const {
            return m_debugInfo;
        }
#endif

    private:

        [[nodiscard]] SKMP_FORCEINLINE const positionData_t& GetParentWorldData() const 
        {
            if (m_scParent) {
                return m_scParent->m_wdObject;
            }
            else {
                return m_wdParent;
            }
        }

        btMatrix3x3 m_itrInitialRot;
        //btMatrix3x3 m_itrMatParent;

        btMatrix3x3 m_nodeRotation;
        btVector3 m_nodePosition;
        rotationParams_t m_rotParams;

        btVector3 m_itrInitialPos;
        //btVector3 m_itrPosParent;

        btVector3 m_gravityCorrection;

        btVector3 m_oldWorldPos;
        btVector3 m_virtld;
        btVector3 m_ld;
        btVector3 m_velocity;
        //btVector3 m_angularVelocity;

        btVector3 m_colExtent;
        btVector3 m_colOffset;

        //Bullet::btBound m_bound;

        NiTransform m_initialTransform;

        btScalar m_colRad;
        btScalar m_colHeight;
        btScalar m_nodeScale;
        btScalar m_invMass;
        btScalar m_maxVelocity2;
        btScalar m_gravForce;

        uint64_t m_groupId;

        configComponent_t m_conf;

        bool m_collisions;
        bool m_motion;

        bool m_resistanceOn;
        bool m_rotScaleOn;
        bool m_hasScaleOverride;
        bool m_hasRotationOverride;
        bool m_hasPositionOverride;
        bool m_hasSpringSlack;
        bool m_hasFriction;

        //bool m_hasBound;

        NiPointer<NiAVObject> m_obj;
        NiPointer<NiNode> m_objParent;
        NiPointer<NiNode> m_objParentOriginal;

        stl::fixed_string m_nodeName;
        stl::fixed_string m_configGroupName;

        stl::queue_simd<Force> m_applyForceQueue;

#ifdef _CBP_ENABLE_DEBUG
        SimDebugInfo m_debugInfo;
#endif

        Game::FormID m_formid;

#if 0
        mutable btSpinMutex m_mutex;
#endif

        Collider m_collider;

        SimObject& m_parent;
    };

    void SimComponent::UpdateVelocity(float a_timeStep)
    {
        if (m_motion)
            return;

        m_velocity = (m_wdObject.m_position - m_oldWorldPos) /= a_timeStep;
        m_oldWorldPos = m_wdObject.m_position;
    }

    SimComponent::rotationParams_t::rotationParams_t() {
        Zero();
    }

    void SimComponent::rotationParams_t::Zero()
    {
        m_axis.setValue(1.0f, 0.0f, 0.0f);
        m_angle = 0.0f;
    }

    void SimComponent::ReadTransforms()
    {
        auto obj = m_obj.get();

        m_wdObject.m_rotation[0].set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[0]), btvFFF0fMask));
        m_wdObject.m_rotation[1].set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[1]), btvFFF0fMask));
        m_wdObject.m_rotation[2].set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[2]), btvFFF0fMask));
        m_wdObject.m_position.set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.pos), btvFFF0fMask));

        if (!m_scParent)
        {
            obj = m_objParent.get();

            m_wdParent.m_rotation[0].set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[0]), btvFFF0fMask));
            m_wdParent.m_rotation[1].set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[1]), btvFFF0fMask));
            m_wdParent.m_rotation[2].set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[2]), btvFFF0fMask));
            m_wdParent.m_position.set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.pos), btvFFF0fMask));
        }
    }
    
    void SimComponent::WriteTransforms()
    {
        auto obj = m_obj.get();

        if (m_motion)
        {
            if (m_rotScaleOn || m_hasRotationOverride)
            {
                _mm_storeu_ps(obj->m_localTransform.rot.data[0], m_ldObject.m_rotation[0].get128());
                _mm_storeu_ps(obj->m_localTransform.rot.data[1], m_ldObject.m_rotation[1].get128());
                _mm_storeu_ps(obj->m_localTransform.rot.data[2], m_ldObject.m_rotation[2].get128());
            }

            obj->m_localTransform.pos.x = m_ldObject.m_position.x();
            obj->m_localTransform.pos.y = m_ldObject.m_position.y();
            obj->m_localTransform.pos.z = m_ldObject.m_position.z();

        }

        //obj->UpdateWorldData(&m_updateCtx);
    }

}