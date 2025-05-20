#include "BattleFrameFunctionLibrary.h"
#include "AssetToolsModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "NiagaraSubjectRenderer.h"
#include "Engine/StaticMesh.h"
#include "NiagaraSystem.h"

UClass* UBattleFrameFunctionLibrary::DuplicateClassAsset(
    UObject* WorldContextObject,
    TSubclassOf<UObject> SourceClass,
    FString NewClassName,
    FString PackagePath)
{
    // 验证输入有效性
    if (!SourceClass || NewClassName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid input parameters!"));
        return nullptr;
    }

    // 确保在游戏主线程执行
    if (!IsInGameThread())
    {
        UE_LOG(LogTemp, Warning, TEXT("Must be called from game thread!"));
        return nullptr;
    }

    // 创建完整资源路径
    const FString FullPackagePath = FPaths::Combine(
        PackagePath,
        NewClassName
    );

    // 检查资源是否已存在
    if (FindObject<UPackage>(nullptr, *FullPackagePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Asset already exists: %s"), *FullPackagePath);
        return nullptr;
    }

    // 获取AssetTools模块
    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

    // 创建复制参数
    FString OutError;
    UObject* NewAsset = AssetToolsModule.Get().DuplicateAsset(
        NewClassName,
        PackagePath,
        SourceClass->ClassGeneratedBy
    );

    if (UBlueprint* NewBlueprint = Cast<UBlueprint>(NewAsset))
    {
        // 强制编译新蓝图
        FKismetEditorUtilities::CompileBlueprint(NewBlueprint);

        // 返回新生成的类
        return NewBlueprint->GeneratedClass;
    }

    return nullptr;
}

void UBattleFrameFunctionLibrary::SetClassDefaultProperties(
    UObject* WorldContextObject,
    TSubclassOf<ANiagaraSubjectRenderer> TargetClass,
    UStaticMesh* NewMesh,
    UNiagaraSystem* NewNiagaraSystem,
    int32 SubType)
{
    if (!TargetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid target class!"));
        return;
    }

    ANiagaraSubjectRenderer* CDO = TargetClass->GetDefaultObject<ANiagaraSubjectRenderer>();
    if (!CDO)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get class default object"));
        return;
    }

    // 设置静态网格（可选参数）
    if (NewMesh)
    {
        CDO->StaticMeshAsset = NewMesh;
        UE_LOG(LogTemp, Log, TEXT("StaticMesh updated to: %s"), *NewMesh->GetName());
    }

    // 设置Niagara系统（可选参数）
    if (NewNiagaraSystem)
    {
        CDO->NiagaraSystemAsset = NewNiagaraSystem;
        UE_LOG(LogTemp, Log, TEXT("NiagaraSystemAsset updated to: %s"), *NewNiagaraSystem->GetName());
    }

    if (NewNiagaraSystem)
    {
        CDO->SubType.Index = SubType;
        UE_LOG(LogTemp, Log, TEXT("NiagaraSystemAsset updated to: %s"), *NewNiagaraSystem->GetName());
    }

    // 标记修改并重新编译
    CDO->MarkPackageDirty();

    if (UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(TargetClass))
    {
        if (UBlueprint* Blueprint = Cast<UBlueprint>(BPClass->ClassGeneratedBy))
        {
            // 使用带验证的编译方式
            FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipGarbageCollection);
            UE_LOG(LogTemp, Log, TEXT("Blueprint recompiled: %s"), *Blueprint->GetName());
        }
    }
}
