#pragma once
#include "SKSEMenuFramework.hpp"

namespace Util::UI {

    // Single item registration: (name -> render fn)
    class UIItemRegistry {
    public:
        using RenderFn = SKSEMenuFramework::Model::RenderFunction;

        //Run on kDataLoaded: SKSEMenuFramework.dll may load after this plugin, so its exports
        //are not resolvable during SKSEPluginLoad.
        static void Install() {
            if (Items().empty()) {
                return;
            }

            if (!GetMenuFrameworkModule()) {
                logger::info("SKSEMenuFramework not loaded, skipping UI registration.");
                return;
            }

            SKSEMenuFramework::SetSection(std::string(SKSE::PluginDeclaration::GetSingleton()->GetName()));
            for (auto const& [name, fn] : Items()) {
                SKSEMenuFramework::AddSectionItem(std::string(name), fn);
            }
        }

        template <class T>
        static void RegisterType() {
            static_assert(requires { T::UICategoryName; });
            static_assert(requires { T::Draw; });

            Items().emplace(T::UICategoryName, RenderFn{ T::Draw });
        }

    private:
        static absl::btree_map<std::string_view, RenderFn>& Items() {
            static absl::btree_map<std::string_view, RenderFn> map;
            return map;
        }
    };

    template <class Derived>
    struct UIEntry {
        static void RegisterUI() { UIItemRegistry::RegisterType<Derived>(); }
    };

}
