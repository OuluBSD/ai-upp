# Task 0000 - RibbonCtrl Skeleton

## Goal
Provide a RibbonCtrl wrapper that loads an XML spec, builds tabs/groups, and can be embedded in ModelerApp.

## Scope
- Create RibbonCtrl with a RibbonBar child.
- Load the CopperCube ribbon XML spec from share.
- Build tabs, groups, buttons, dropdowns, and basic form rows.
- Expose action callback for button presses.

## Acceptance
- Ribbon loads on ModelerApp startup without errors.
- Tabs/groups appear based on the XML.
- Buttons invoke a callback with the correct action id.
