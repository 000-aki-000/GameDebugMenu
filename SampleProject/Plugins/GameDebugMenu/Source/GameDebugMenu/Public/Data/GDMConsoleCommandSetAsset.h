/**
* Copyright (c) 2020 akihiko moroi
*
* This software is released under the MIT License.
* (See accompanying file LICENSE.txt or copy at http://opensource.org/licenses/MIT)
*/

#pragma once

#include "CoreMinimal.h"
#include "GameDebugMenuTypes.h"
#include "Engine/DataAsset.h"
#include "GDMConsoleCommandSetAsset.generated.h"

/**
 * デバッグメニューで使用可能なコンソールコマンド群を定義するためのデータアセット
 * AGameDebugMenuManager によって参照され、ユーザーがゲーム中にデバッグUIを通じてコマンドを実行できるようにするものです
 * コマンド情報はINIファイルで設定されたものを一括追加、他アセットのマージなど可能
 */
UCLASS(Const, BlueprintType)
class GAMEDEBUGMENU_API UGDMConsoleCommandSetAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/** コンソールコマンド名 */
	UPROPERTY(EditAnywhere, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandSingle> ConsoleCommandNames;
	
	/** コンソールコマンドグループ名（同時に複数コマンド実行） */
	UPROPERTY(EditAnywhere, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandGroup> ConsoleCommandGroups;

	/** コンソールコマンドペア名（トグルで２種のコマンド実行） */
	UPROPERTY(EditAnywhere, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandPair> ConsoleCommandPairs;

	/** コンソールコマンドナンバー（数値設定コマンド実行） */
	UPROPERTY(EditAnywhere, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandNumber> ConsoleCommandNumbers;

	/** エディターでのみ追加されるコンソールコマンド名 */
	UPROPERTY(EditAnywhere, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandSingle> EditorOnlyConsoleCommandNames;

	/** エディターでのみ追加されるコンソールコマンドグループ名 */
	UPROPERTY(EditAnywhere, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandGroup> EditorOnlyConsoleCommandGroups;

	/** エディターでのみ追加されるコンソールコマンドペア名 */
	UPROPERTY(EditAnywhere, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandPair> EditorOnlyConsoleCommandPairs;

	/** エディターでのみ追加されるコンソールコマンドペア名 */
	UPROPERTY(EditAnywhere, Category = "ConsoleCommand")
	TArray<FGDMConsoleCommandNumber> EditorOnlyConsoleCommandNumbers;

	/** コンソールコマンドのカテゴリ名表示順 */
	UPROPERTY(EditAnywhere, EditFixedSize, Category = "ConsoleCommand")
	TArray<FGDMOrderMenuCategoryTitle> OrderConsoleCommandCategoryTitles;

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Editor")
	TArray<int32> CategoryIndexList;

	/** マージ対象の外部アセット */
	UPROPERTY(EditAnywhere, Category = "Editor")
	TArray<UGDMConsoleCommandSetAsset*> MergeSourceAssets;
#endif

	/**
	 * INIファイルで設定されたコマンド郡をこのアセットにセットする
	 */
	UFUNCTION(CallInEditor, Category = "Editor")
	void SetupCommandNames();

	/**
	 * INIファイルで設定されたコマンドのカテゴリ順をこのアセットにセットする
	 */
	UFUNCTION(CallInEditor, Category = "Editor")
	void SetupOrderConsoleCommandCategoryTitles();
	
	/**
	 * アセットをマージする
	 */
	UFUNCTION(CallInEditor, Category = "Editor")
	void MergeFromSourceAssets();

public:
	/**
	* コンソールコマンドメニューのカテゴリ名を取得
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	TArray<FGDMMenuCategoryKey> GetOrderConsoleCommandCategoryTitle() const;

	/** 
	* コンソールコマンドの情報取得
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	bool GetConsoleCommandNameByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandSingle& Out) const;

	/**
	* コンソールコマンド数
	*/
	UFUNCTION(BlueprintPure)
	int32 GetNumConsoleCommandNames() const;

	/** 
	* コンソールコマンドグループの情報取得
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	bool GetConsoleCommandGroupByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandGroup& Out) const;

	/**
	* コンソールコマンドグループ数
	*/
	UFUNCTION(BlueprintPure)
	int32 GetNumConsoleCommandGroups() const;

	/** 
	* コンソールコマンドペアの情報取得
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	bool GetConsoleCommandPairByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandPair& Out) const;

	/**
	* コンソールコマンドペア数
	*/
	UFUNCTION(BlueprintPure)
	int32 GetNumConsoleCommandPairs() const;

	/**
	* コンソールコマンドナンバーの情報取得
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	bool GetConsoleCommandNumberByArrayIndex(const int32 ArrayIndex, FGDMConsoleCommandNumber& Out) const;

	/**
	* コンソールコマンドナンバー数
	*/
	UFUNCTION(BlueprintPure)
	int32 GetNumConsoleCommandNumbers() const;

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	bool FindConsoleCommandSingleById(const FString& CommandId, FGDMConsoleCommandSingle& Out) const;

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	bool FindConsoleCommandGroupById(const FString& CommandId, FGDMConsoleCommandGroup& Out) const;

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	bool FindConsoleCommandPairById(const FString& CommandId, FGDMConsoleCommandPair& Out) const;

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	bool FindConsoleCommandNumberById(const FString& CommandId, FGDMConsoleCommandNumber& Out) const;
};
