Сборка начата в 23:12...
1>------ Сборка начата: проект: Depthrun, Конфигурация: Development_Editor x64 ------
1>  Using bundled DotNet SDK version: 8.0.412 win-x64
1>  Running UnrealBuildTool: dotnet "..\..\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll" DepthrunEditor Win64 Development -Project="E:\Projects\Unreal Projects\Depthrun\Depthrun.uproject" -WaitMutex -FromMsBuild -architecture=x64
1>  Log file: C:\Users\mrxop\AppData\Local\UnrealBuildTool\Log.txt
1>  Using 'git status' to determine working set for adaptive non-unity build (E:\Projects\Unreal Projects\Depthrun).
1>  Building DepthrunEditor...
1>  Using Visual Studio 2026 14.44.35225 toolchain (C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.44.35207) and Windows 10.0.22621.0 SDK (C:\Program Files (x86)\Windows Kits\10).
1>  [Adaptive Build] Excluded from Depthrun unity file: BaseProjectile.cpp, BaseWeapon.cpp, MeleeWeapon.cpp, RangedWeapon.cpp, DepthrunGameInstance.cpp, DepthrunGameMode.cpp, DepthrunLogChannels.cpp, DepthrunCharacter.cpp, PlayerActionTracker.cpp, PlayerCombatComponent.cpp, MathUtils.cpp
1>  [Adaptive Build] Excluded from VisualStudioTools unity file: BlueprintAssetHelper.cpp, BlueprintReferencesCommandlet.cpp, VisualStudioTools.cpp, VisualStudioToolsBlueprintBreakpointExtension.cpp, VisualStudioToolsCommandlet.cpp, VisualStudioToolsCommandletBase.cpp, VSServerCommandlet.cpp, VSTestAdapterCommandlet.cpp
1>  [Adaptive Build] Excluded from VisualStudioBlueprintDebuggerHelper unity file: VisualStudioBlueprintDebuggerHelperModule.cpp
1>  Determining max actions to execute in parallel (6 physical cores, 6 logical cores)
1>    Executing up to 6 processes, one per physical core
1>    Requested 1.5 GB memory per action, 6.05 GB available: limiting max parallel actions to 4
1>  Using Unreal Build Accelerator local executor to run 33 action(s)
1>    Storage capacity 40Gb
1>  ---- Starting trace: 260402_231304_vs21080 ----
1>  UbaServer - Listening on 0.0.0.0:1345
1>  [1/33] Compile [x64] Depthrun.cpp
1>  [2/33] Compile [x64] BaseWeapon.cpp
1>  [3/33] Compile [x64] DepthrunGameInstance.cpp
1>  [4/33] Compile [x64] DepthrunLogChannels.cpp
1>  [5/33] Compile [x64] MathUtils.cpp
1>  [6/33] Compile [x64] BaseProjectile.cpp
1>E:\Projects\Unreal Projects\Depthrun\Source\Depthrun\Combat\BaseProjectile.cpp(54,51): error C2027: использование неопределенного типа "FDamageEvent"
1>  	OtherActor->TakeDamage(DamageAmount, FDamageEvent(), nullptr, ShooterActor);
1>  	                                                 ^
1>  E:\Programs\Unreal Engine\UE_5.7\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h(3659,65): note:  см. объявление "FDamageEvent"
1>  	ENGINE_API virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);
1>  	                                                               ^
1>E:\Projects\Unreal Projects\Depthrun\Source\Depthrun\Combat\BaseProjectile.cpp(54,14): error C2660: AActor::TakeDamage: функция не принимает 3 аргументов
1>  	OtherActor->TakeDamage(DamageAmount, FDamageEvent(), nullptr, ShooterActor);
1>  	            ^
1>  E:\Programs\Unreal Engine\UE_5.7\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h(3659,27): note:  см. объявление "AActor::TakeDamage"
1>  	ENGINE_API virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);
1>  	                         ^
1>  E:\Projects\Unreal Projects\Depthrun\Source\Depthrun\Combat\BaseProjectile.cpp(54,14): note: при попытке сопоставить список аргументов "(float, nullptr, TObjectPtr<AActor>)"
1>  	OtherActor->TakeDamage(DamageAmount, FDamageEvent(), nullptr, ShooterActor);
1>  	            ^
1>  [7/33] Compile [x64] MeleeWeapon.cpp
1>E:\Projects\Unreal Projects\Depthrun\Source\Depthrun\Combat\MeleeWeapon.cpp(27,69): error C2248: ABaseWeapon::OnCooldownEnd: невозможно обратиться к protected член, объявленному в классе "ABaseWeapon"
1>  	GetWorldTimerManager().SetTimer(CooldownTimer, this, &ABaseWeapon::OnCooldownEnd, AttackCooldown, false);
1>  	                                                                   ^
1>  E:\Projects\Unreal Projects\Depthrun\Source\Depthrun\Combat\BaseWeapon.h(49,7): note:  см. объявление "ABaseWeapon::OnCooldownEnd"
1>  	void OnCooldownEnd();
1>  	     ^
1>  E:\Projects\Unreal Projects\Depthrun\Source\Depthrun\Combat\BaseWeapon.h(22,20): note:  см. объявление "ABaseWeapon"
1>  class DEPTHRUN_API ABaseWeapon : public AActor
1>                     ^
1>E:\Projects\Unreal Projects\Depthrun\Source\Depthrun\Combat\MeleeWeapon.cpp(51,49): error C2027: использование неопределенного типа "FDamageEvent"
1>  	OtherActor->TakeDamage(BaseDamage, FDamageEvent(), nullptr, GetOwner());
1>  	                                               ^
1>  E:\Programs\Unreal Engine\UE_5.7\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h(3659,65): note:  см. объявление "FDamageEvent"
1>  	ENGINE_API virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);
1>  	                                                               ^
1>E:\Projects\Unreal Projects\Depthrun\Source\Depthrun\Combat\MeleeWeapon.cpp(51,14): error C2660: AActor::TakeDamage: функция не принимает 3 аргументов
1>  	OtherActor->TakeDamage(BaseDamage, FDamageEvent(), nullptr, GetOwner());
1>  	            ^
1>  E:\Programs\Unreal Engine\UE_5.7\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h(3659,27): note:  см. объявление "AActor::TakeDamage"
1>  	ENGINE_API virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);
1>  	                         ^
1>  E:\Projects\Unreal Projects\Depthrun\Source\Depthrun\Combat\MeleeWeapon.cpp(51,14): note: при попытке сопоставить список аргументов "(float, nullptr, AActor *)"
1>  	OtherActor->TakeDamage(BaseDamage, FDamageEvent(), nullptr, GetOwner());
1>  	            ^
1>  [8/33] Compile [x64] PerModuleInline.gen.cpp
1>  [9/33] Compile [x64] DepthrunGameMode.cpp
1>  [10/33] Compile [x64] PlayerActionTracker.cpp
1>  [11/33] Compile [x64] PlayerCombatComponent.cpp
1>  [12/33] Compile [x64] RangedWeapon.cpp
1>E:\Projects\Unreal Projects\Depthrun\Source\Depthrun\Combat\RangedWeapon.cpp(39,69): error C2248: ABaseWeapon::OnCooldownEnd: невозможно обратиться к protected член, объявленному в классе "ABaseWeapon"
1>  	GetWorldTimerManager().SetTimer(CooldownTimer, this, &ABaseWeapon::OnCooldownEnd, AttackCooldown, false);
1>  	                                                                   ^
1>  E:\Projects\Unreal Projects\Depthrun\Source\Depthrun\Combat\BaseWeapon.h(49,7): note:  см. объявление "ABaseWeapon::OnCooldownEnd"
1>  	void OnCooldownEnd();
1>  	     ^
1>  E:\Projects\Unreal Projects\Depthrun\Source\Depthrun\Combat\BaseWeapon.h(22,20): note:  см. объявление "ABaseWeapon"
1>  class DEPTHRUN_API ABaseWeapon : public AActor
1>                     ^
1>  [13/33] Compile [x64] Module.Depthrun.gen.cpp
1>  [14/33] Compile [x64] BlueprintAssetHelper.cpp
1>  [15/33] Compile [x64] BlueprintReferencesCommandlet.cpp
1>  [16/33] Resource Default.rc2
1>  [17/33] Compile [x64] Module.VisualStudioBlueprintDebuggerHelper.cpp
1>  [18/33] Compile [x64] DepthrunCharacter.cpp
1>  [19/33] Compile [x64] VSTestAdapterCommandlet.cpp
1>  [20/33] Compile [x64] VisualStudioTools.cpp
1>  [21/33] Compile [x64] Module.VisualStudioTools.cpp
1>  [22/33] Compile [x64] VSServerCommandlet.cpp
1>  [23/33] Compile [x64] VisualStudioToolsCommandlet.cpp
1>  [24/33] Compile [x64] VisualStudioToolsCommandletBase.cpp
1>  [25/33] Compile [x64] VisualStudioToolsBlueprintBreakpointExtension.cpp
1>  [26/33] Link [x64] UnrealEditor-VisualStudioTools.lib
1>     Создается библиотека E:/Projects/Unreal Projects/Depthrun/Plugins/VisualStudioTools/Intermediate/Build/Win64/x64/UnrealEditor/Development/VisualStudioTools/UnrealEditor-VisualStudioTools.lib и объект E:/Projects/Unreal Projects/Depthrun/Plugins/VisualStudioTools/Intermediate/Build/Win64/x64/UnrealEditor/Development/VisualStudioTools/UnrealEditor-VisualStudioTools.exp
1>  [27/33] Link [x64] UnrealEditor-VisualStudioTools.dll
1>  [28/33] Compile [x64] VisualStudioBlueprintDebuggerHelperModule.cpp
1>  [29/33] Link [x64] UnrealEditor-VisualStudioBlueprintDebuggerHelper.lib
1>     Создается библиотека E:/Projects/Unreal Projects/Depthrun/Plugins/VisualStudioTools/Intermediate/Build/Win64/x64/UnrealEditor/Development/VisualStudioBlueprintDebuggerHelper/UnrealEditor-VisualStudioBlueprintDebuggerHelper.lib и объект E:/Projects/Unreal Projects/Depthrun/Plugins/VisualStudioTools/Intermediate/Build/Win64/x64/UnrealEditor/Development/VisualStudioBlueprintDebuggerHelper/UnrealEditor-VisualStudioBlueprintDebuggerHelper.exp
1>  [30/33] Link [x64] UnrealEditor-VisualStudioBlueprintDebuggerHelper.dll
1>  Trace written to file C:/Users/mrxop/AppData/Local/UnrealBuildTool/Log.uba with size 74.3kb
1>  Total time in Unreal Build Accelerator local executor: 712.67 seconds
1>
1>  Result: Failed (OtherCompilationError)
1>  Total execution time: 722.75 seconds
1>C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Microsoft\VC\v180\Microsoft.MakeFile.Targets(44,5): error MSB3073: выход из команды ""E:\Programs\Unreal Engine\UE_5.7\Engine\Build\BatchFiles\Build.bat" DepthrunEditor Win64 Development -Project="E:\Projects\Unreal Projects\Depthrun\Depthrun.uproject" -WaitMutex -FromMsBuild -architecture=x64" с кодом 6.
========== Сборка: успешно выполнено — 0 , со сбоем — 1, в актуальном состоянии — 0, пропущено — 0 ==========
========== Сборка завершено в 23:24 и заняло 12:06,334 мин ==========