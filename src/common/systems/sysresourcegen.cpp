/*
* Copyright 2021 Conquer Space
*/
#include "common/systems/sysresourcegen.h"

#include <spdlog/spdlog.h>

#include "common/components/area.h"
#include "common/components/economy.h"
#include "common/components/resource.h"

void conquerspace::common::systems::SysResourceGen::DoSystem(components::Universe& universe) {
    namespace cqspc = conquerspace::common::components;

    // Tick date
    auto resource_generator = universe.view<cqspc::ResourceGenerator, cqspc::ResourceStockpile>();

    for (entt::entity entity : resource_generator) {
        // Make resources
        cqspc::ResourceGenerator &generator = universe.get<cqspc::ResourceGenerator>(entity);
        cqspc::ResourceStockpile &resource_stockpile =
                                                universe.get<cqspc::ResourceStockpile>(entity);

        float productivity = 1;
        if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
        }

        resource_stockpile += generator * productivity * Interval();
    }
}

void conquerspace::common::systems::SysFactoryResourceProduction::DoSystem(
                                                                components::Universe &universe) {
    namespace cqspc = conquerspace::common::components;
    auto view = universe.view<cqspc::Production, cqspc::ResourceConverter,
                                                                    cqspc::ResourceStockpile>();
    for (entt::entity entity : view) {
        // Do the same thing
        // Make resources
        cqspc::ResourceConverter &generator = universe.get<cqspc::ResourceConverter>(entity);
        cqspc::ResourceStockpile &resource_stockpile =
                                      universe.get<cqspc::ResourceStockpile>(entity);
        cqspc::Recipe &recipe = universe.get<cqspc::Recipe>(generator.recipe);

        float productivity = 1;
        if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
        }

        resource_stockpile += (recipe.output *productivity* Interval());
    }
}

void conquerspace::common::systems::SysFactoryResourceConsumption::DoSystem(
                                                                components::Universe &universe) {
    namespace cqspc = conquerspace::common::components;

    // Consume resources
    auto consume = universe.view<cqspc::ResourceConverter, cqspc::ResourceStockpile, cqspc::MarketParticipant>();
    for (entt::entity entity : consume) {
        // Do the same thing
        // Make resources
        cqspc::ResourceConverter &generator = universe.get<cqspc::ResourceConverter>(entity);
        cqspc::ResourceStockpile &resource_stockpile =
                                    universe.get<cqspc::ResourceStockpile>(entity);
        cqspc::Recipe &recipe = universe.get<cqspc::Recipe>(generator.recipe);

        float productivity = 1;
        if (universe.all_of<cqspc::FactoryProductivity>(entity)) {
            productivity = universe.get<cqspc::FactoryProductivity>(entity).productivity;
        }
        // Check if attached to market, and buy from market
        auto& participant = universe.get<cqspc::MarketParticipant>(entity);
        // Get market and stuff
        auto &market_stockpile =
            universe.get<cqspc::ResourceStockpile>(participant.market);
        //If it's enough, add production
        if (market_stockpile > resource_stockpile) {
            universe.emplace_or_replace<cqspc::Production>(entity);
            market_stockpile -= (recipe.input * productivity * Interval());
        }
    }
}