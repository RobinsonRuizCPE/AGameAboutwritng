#include "AGameAboutWrittingEditor.h"
#include "ItemBlueprintFixer.h"
#include "HAL/IConsoleManager.h"

void FAGameAboutWrittingEditorModule::StartupModule()
{
    // Register console command
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("FixItems"),
        TEXT("Fixes all item blueprints"),
        FConsoleCommandDelegate::CreateLambda([]()
        {
            FItemBlueprintFixer::FixAllItemBlueprints();
        }),
        ECVF_Default
    );

    UE_LOG(LogTemp, Warning, TEXT("AGameAboutWrittingEditor module loaded."));
}

void FAGameAboutWrittingEditorModule::ShutdownModule()
{
    // Nothing special
}

IMPLEMENT_MODULE(FAGameAboutWrittingEditorModule, AGameAboutWrittingEditor)
