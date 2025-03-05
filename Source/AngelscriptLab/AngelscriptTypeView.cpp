// Fill out your copyright notice in the Description page of Project Settings.


#include "AngelscriptTypeView.h"

#include "AngelscriptType.h"
#include "imgui.h"


// Sets default values
AAngelscriptTypeView::AAngelscriptTypeView()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAngelscriptTypeView::BeginPlay()
{
	// auto Types = FAngelscriptType::GetTypes();
	//
	// for (auto& Type : Types)
	// {
	// 	FString AngelscriptName = Type->GetAngelscriptTypeName();
	// 	AngelscriptTypes.Add(AngelscriptName);
	// }
}

// Called every frame
void AAngelscriptTypeView::Tick(float DeltaTime)
{
	static char FilterText[128] = "";
	static int FilteredCount = 0;
	
	auto AngelscriptTypes = FAngelscriptType::GetTypes();


	ImGui::Begin("AngelScript Types");

	// 搜索过滤框
	ImGui::InputText("Filter", FilterText, IM_ARRAYSIZE(FilterText));

	// // 自适应滚动区域
	// ImGui::BeginChild("TableScroll", ImVec2(0, 0), ImGuiChildFlags_FrameStyle);
	// 计算可用高度（总高度 - 搜索框高度 - 统计信息高度 - 间距）
	const float footerHeight = ImGui::GetFrameHeightWithSpacing();
	ImGui::BeginChild("TableScroll", ImVec2(0, -footerHeight), ImGuiChildFlags_FrameStyle);


	if (ImGui::BeginTable("TypeTable", 1,
	                      ImGuiTableFlags_Borders |
	                      ImGuiTableFlags_RowBg |
	                      ImGuiTableFlags_SizingFixedFit))
	{
		ImGui::TableSetupColumn("Type Name", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		const FString FilterString = FString(UTF8_TO_TCHAR(FilterText)).ToLower();
		FilteredCount = 0;

		// 预计算过滤数量
		for (auto& Type : AngelscriptTypes)
		{
			const FString& TypeName = Type->GetAngelscriptTypeName(); 
			if (FilterString.IsEmpty() || TypeName.ToLower().Contains(FilterString))
			{
				FilteredCount++;
			}
		}

		// 优化渲染性能
		ImGuiListClipper clipper;
		clipper.Begin(FilteredCount);
		while (clipper.Step())
		{
			int actualIndex = 0;
			for (int i = 0; i < AngelscriptTypes.Num() && actualIndex < clipper.DisplayEnd; ++i)
			{
				auto& Type = AngelscriptTypes[i];
				const FString& TypeName = Type->GetAngelscriptTypeName();
				if (!FilterString.IsEmpty() && !TypeName.ToLower().Contains(FilterString))
				{
					continue;
				}

				if (actualIndex >= clipper.DisplayStart)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("%s", TCHAR_TO_ANSI(*TypeName));
				}
				actualIndex++;
			}
		}

		ImGui::EndTable();
	}
	ImGui::EndChild();

	// 显示统计信息
	ImGui::Text("Total: %d  Filtered: %d", AngelscriptTypes.Num(), FilteredCount);
	
	ImGui::End();
}

