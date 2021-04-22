#pragma once

namespace Bullet
{
    ATTRIBUTE_ALIGNED16(class)
        btBound
    {
    public:
        btBound() = default;

        SIMD_FORCE_INLINE btBound(
            const btVector3 & a_pos,
            btScalar a_radius)
            :
            m_pos(a_pos),
            m_radius(a_radius)
        {
        }

        SIMD_FORCE_INLINE btBound(
            const NiPoint3 & a_pos,
            btScalar a_radius)
            :
            m_pos(a_pos.x, a_pos.y, a_pos.z),
            m_radius(a_radius)
        {
        }

        SIMD_FORCE_INLINE btBound(
            const NiBound & a_bound)
            :
            m_radius(a_bound.radius),
            m_pos(_mm_and_ps(_mm_loadu_ps(a_bound.pos), btvFFF0fMask))
        {
        }

        btVector3 m_pos;
        btScalar m_radius;
    };

    ATTRIBUTE_ALIGNED16(class)
        btTransformEx : public btTransform
    {
        btScalar m_scale;

    public:

        using btTransform::btTransform;
        using btTransform::operator=;

        explicit SIMD_FORCE_INLINE btTransformEx(
            const NiTransform& a_tf)
        {
            _copy_from_nitransform(a_tf);
        }

        SIMD_FORCE_INLINE btTransformEx& operator=(const NiTransform& a_tf)
        {
            _copy_from_nitransform(a_tf);
            return *this;
        }

        SIMD_FORCE_INLINE btVector3 btTransformEx::operator*(const btVector3& a_pt) const
        {
            return ((getBasis() * a_pt) *= m_scale) += getOrigin();
        }

        SIMD_FORCE_INLINE btScalar getScale() const {
            return m_scale;
        }

        SIMD_FORCE_INLINE void setScale(btScalar a_scale) {
            m_scale = a_scale;
        }

    private:

        SIMD_FORCE_INLINE void _copy_from_nitransform(const NiTransform& a_tf)
        {
            auto& b = getBasis();

            b[0].set128(_mm_and_ps(_mm_loadu_ps(a_tf.rot.data[0]), btvFFF0fMask));
            b[1].set128(_mm_and_ps(_mm_loadu_ps(a_tf.rot.data[1]), btvFFF0fMask));
            b[2].set128(_mm_and_ps(_mm_loadu_ps(a_tf.rot.data[2]), btvFFF0fMask));

            getOrigin().set128(_mm_and_ps(_mm_loadu_ps(a_tf.pos), btvFFF0fMask));

            m_scale = a_tf.scale;
        }
    };

}