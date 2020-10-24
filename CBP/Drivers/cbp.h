#pragma once

namespace CBP
{
    class DCBP :
        ILog,
        IConfigINI
    {
        
        enum SerializationVersion : UInt32 {
            kDataVersion1 = 1,
            kDataVersion2 = 2,
        };

        class KeyPressHandler : public KeyEventHandler
        {
        public:
            virtual void ReceiveEvent(KeyEvent, UInt32) override;

            void UpdateKeys();
        private:

            bool combo_down = false;
            bool combo_downDR = false;

            UInt32 m_comboKey;
            UInt32 m_showKey;

            UInt32 m_comboKeyDR;
            UInt32 m_showKeyDR;
        };

        class ToggleUITask :
            public TaskDelegateStatic
        {
            enum class ToggleResult
            {
                kResultNone,
                kResultEnabled,
                kResultDisabled
            };
        public:
            virtual void Run();
        private:
            ToggleResult Toggle();
        };

        class SwitchUITask :
            public TaskDelegate
        {
        public:
            SwitchUITask(bool a_switch) :
                m_switch(a_switch)
            {
            }

            virtual void Run();
            virtual void Dispose() {
                delete this;
            };
        private:
            bool m_switch;
        };

        class ApplyForceTask :
            public TaskDelegate
        {
        public:
            ApplyForceTask(
                Game::ObjectHandle a_handle,
                uint32_t a_steps,
                const std::string& a_component,
                const NiPoint3& a_force
            );

            virtual void Run();
            virtual void Dispose() {
                delete this;
            }
        private:
            Game::ObjectHandle m_handle;
            uint32_t m_steps;
            std::string m_component;
            NiPoint3 m_force;
        };

        class UpdateActorCacheTask :
            public TaskDelegateStatic
        {
        public:
            virtual void Run();
        };

        typedef void (*mainLoopUpdateFunc_t)(Game::BSMain *a_main);

    public:

        static bool LoadPaths();
        static void Initialize();

        static void MainLoop_Hook(Game::BSMain* a_main);
        static void OnCreateArmorNode(TESObjectREFR* a_ref, BipedParam* a_params);
        static NiAVObject* CreateArmorNode_Hook(NiAVObject* a_obj, Biped* a_info, BipedParam* a_params);

        static void DispatchActorTask(Actor* actor, ControllerInstruction::Action action);
        static void DispatchActorTask(Game::ObjectHandle handle, ControllerInstruction::Action action);

        [[nodiscard]] inline static const auto& GetSimActorList() {
            return m_Instance.m_updateTask.GetSimActorList();
        }

        static void UpdateConfigOnAllActors();
        static void UpdateGroupInfoOnAllActors();
        static void ResetPhysics();
        static void NiNodeUpdate();
        static void NiNodeUpdate(Game::ObjectHandle a_handle);
        static void WeightUpdate();
        static void WeightUpdate(Game::ObjectHandle a_handle);
        static void ClearArmorOverrides();
        static void UpdateArmorOverridesAll();
        static void ResetActors();
        static void UpdateDebugRendererState();
        static void UpdateDebugRendererSettings();
        static void UpdateProfilerSettings();
        static void ApplyForce(Game::ObjectHandle a_handle, uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force);

        static bool SaveAll();

        inline static bool SaveGlobals() {
            return m_Instance.m_serialization.SaveGlobalConfig();
        }

        inline static void MarkGlobalsForSave() {
            m_Instance.m_serialization.MarkForSave(ISerialization::kGlobals);
        }

        inline static void MarkForSave(ISerialization::Group a_grp) {
            m_Instance.m_serialization.MarkForSave(a_grp);
        }

        inline static bool SaveCollisionGroups() {
            return m_Instance.m_serialization.SaveCollisionGroups();
        }

        inline static bool SavePending() {
            return m_Instance.m_serialization.SavePending();
        }

        inline static bool SaveToDefaultGlobalProfile() {
            return m_Instance.m_serialization.SaveToDefaultGlobalProfile();
        }

        static bool ExportData(const std::filesystem::path& a_path);
        static bool ImportData(const std::filesystem::path& a_path, ISerialization::ImportFlags a_flags);
        static bool GetImportInfo(const std::filesystem::path& a_path, importInfo_t& a_out);

        [[nodiscard]] inline static const auto& GetLastSerializationException() {
            return m_Instance.m_serialization.GetLastException();
        }

        inline static void UIQueueUpdateCurrentActorA() {
            m_Instance.m_uiContext.QueueListUpdateCurrent();
        }

        inline static void QueueActorCacheUpdate() {
            DTasks::AddTask(&m_Instance.m_updateActorCacheTask);
        }

        [[nodiscard]] inline static auto& GetUpdateTask() {
            return m_Instance.m_updateTask;
        }

        static void ResetProfiler();
        static void SetProfilerInterval(long long a_interval);

        [[nodiscard]] inline static auto& GetProfiler() {
            return m_Instance.m_updateTask.GetProfiler();
        }

        inline static void Lock() {
            m_Instance.m_lock.Enter();
        }

        inline static void Unlock() {
            m_Instance.m_lock.Leave();
        }

        inline static auto GetLock() {
            return std::addressof(m_Instance.m_lock);
        }

        [[nodiscard]] inline static const auto& GetDriverConfig()
        {
            return m_Instance.m_conf;
        }

        inline static auto& GetRenderer() {
            return m_Instance.m_renderer;
        }

        inline static void UpdateKeys() {
            m_Instance.m_inputEventHandler.UpdateKeys();
        }

        inline static void SetMarkedActor(Game::ObjectHandle a_handle) {
            m_Instance.m_updateTask.SetMarkedActor(a_handle);
        }

        inline static void QueueUIReset() {
            m_Instance.m_resetUI = true;
        }

        inline static auto& GetUIContext() {
            return m_Instance.m_uiContext;
        }

        FN_NAMEPROC("CBP")
    private:
        DCBP();

        void LoadConfig();

        bool ProcessUICallbackImpl();

        static void MessageHandler(Event m_code, void* args);
        static void OnLogMessage(Event, void* args);
        static void OnExit(Event, void* data);

        static void RevertHandler(Event m_code, void* args);
        static void LoadGameHandler(SKSESerializationInterface* intfc, UInt32 type, UInt32 length, UInt32 version);
        static void SaveGameHandler(Event m_code, void* args);

        static void SerializationStats(UInt32 a_type, const CBP::ISerialization::stats_t& a_stats);

        template <typename T>
        static bool LoadRecord(SKSESerializationInterface* intfc, UInt32 a_type, bool a_bin, T a_func);
        template <typename T>
        static bool SerializeToSave(SKSESerializationInterface* intfc, UInt32 a_type, T a_func);

        static void OnD3D11PostCreate_CBP(Event code, void* data);
        static void Present_Pre();

        static uint32_t ConfigGetComboKey(int32_t param);

        static bool UICallback();

        void EnableUI();
        void DisableUI();
        bool RunEnableUIChecks();

        struct
        {
            bool ui_enabled;
            bool debug_renderer;
            bool force_ini_keys;
            int compression_level;

            bool use_epa;
            int maxPersistentManifoldPoolSize;
            int maxCollisionAlgorithmPoolSize;

            UInt32 comboKey;
            UInt32 showKey;

            struct
            {
                fs::path root;
                fs::path profilesPhysics;
                fs::path profilesNode;
                fs::path settings;
                fs::path collisionGroups;
                fs::path nodes;
                fs::path defaultProfile;
                fs::path exports;
                fs::path templateProfilesPhysics;
                fs::path templateProfilesNode;
                fs::path templatePlugins;
                fs::path colliderData;
            } paths;

            std::string imguiIni;
        } m_conf;

        KeyPressHandler m_inputEventHandler;
        UIContext m_uiContext;
        uint32_t m_loadInstance;

        ControllerTask m_updateTask;
        ToggleUITask m_taskToggle;
        UpdateActorCacheTask m_updateActorCacheTask;

        struct {
            bool show;
            bool lockControls;
        } uiState;

        ISerialization m_serialization;
        std::unique_ptr<Renderer> m_renderer;

        mainLoopUpdateFunc_t mainLoopUpdateFunc_o;

        bool m_resetUI;

        ICriticalSection m_lock;

        static DCBP m_Instance;
    };
}