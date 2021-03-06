#pragma once

namespace CBP
{
    struct ControllerInstruction
    {
        enum class Action : std::uint32_t
        {
            AddActor,
            RemoveActor,
            UpdateConfig,
            UpdateConfigOrAdd,
            UpdateConfigAll,
            Reset,
            PhysicsReset,
            NiNodeUpdate,
            NiNodeUpdateAll,
            WeightUpdate,
            WeightUpdateAll,
            AddArmorOverride,
            UpdateArmorOverride,
            UpdateArmorOverridesAll,
            ClearArmorOverrides,
            ValidateNodes,
            RemoveInvalidNodes
        };

        Action m_action;
        Game::VMHandle m_handle;
    };
};