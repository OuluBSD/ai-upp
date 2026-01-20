////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

namespace DemoRoom 
{
    ////////////////////////////////////////////////////////////////////////////////
    // AppLogicSystem
    // Simple system to destroy objects that get too far away from the user
    class AppLogicSystem : public System
    {
    public:
        SYS_CTOR(AppLogicSystem)

    protected:
        void Update(double /*dt*/) override
        {
            auto& root = GetEngine().GetRootPool();
            auto entities = root.FindAllDeep<Entity>();
            for (auto& entity : entities) {
                auto transform = entity->val.Find<Transform>();
                if (!transform)
                    continue;

                // Destroy any objects that fall too far away (Baseballs and Bullets)
                if (transform->position.y < -10.0f)
                    entity->Destroy();
            }
        }
    };
}
