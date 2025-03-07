class AImGuiExample : AActor
{
    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent Root;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
    }

    UPROPERTY(EditAnywhere)
    bool bWindowOpen = true;

    UPROPERTY()
    FString InputText = "Hello Unreal";

    UPROPERTY()
    int32 SelectedItem = 0;

    UPROPERTY()
    float SliderValue = 0.5f;

    UPROPERTY()
    bool bCheckbox = true;

    UPROPERTY()
    FLinearColor ColorPick = FLinearColor::Red;

    UPROPERTY()
    float DragValue = 1.0f;

    UPROPERTY()
    int32 ComboSelection = 0;

    UPROPERTY()
    TArray<FString> ComboItems;

    AImGuiExample()
    {
        ComboItems.Add("Item 1");
        ComboItems.Add("Item 2");
        ComboItems.Add("Item 3");
    }

    UFUNCTION(BlueprintOverride)
    void Tick(float DeltaTime)
    {
		if (!ImGui::Begin("ImGui Demo Window", bWindowOpen))
		{
			ImGui::End();
			return;
		}

		// 1. 基础控件
		if (ImGui::CollapsingHeader("Basic Widgets", EImGuiTreeNodeFlags::None))
		{
			ImGui::Text("Basic Widgets Section");
			ImGui::Separator();

			// 按钮
			if (ImGui::Button("Click Me!"))
			{
				Print("Button clicked!");
			}

			// 复选框
			ImGui::Checkbox("Checkbox", bCheckbox);

			// 单选框
			// int radio = 0;
			// ImGui::RadioButton("Radio 1", radio, 0);
			// ImGui::SameLine();
			// ImGui::RadioButton("Radio 2", radio, 1);

			// 输入框
			ImGui::InputText("Input", InputText);

			// 滑动条
			ImGui::SliderFloat("Slider", SliderValue, 0.0f, 1.0f);

			// 拖拽控件
			// ImGui::DragFloat("Drag", DragValue, 0.1f);
		}

		// 2. 布局控件
		// if (ImGui::CollapsingHeader("Layout", EImGuiTreeNodeFlags::None))
		// {
		// 	ImGui::Text("Layout Controls");
		// 	ImGui::Separator();

		// 	// 列布局
		// 	ImGui::Columns(2, "mycolumns");
		// 	ImGui::Text("First Column");
		// 	ImGui::NextColumn();
		// 	ImGui::Text("Second Column");
		// 	ImGui::Columns(1);

		// 	// 缩进
		// 	ImGui::Indent();
		// 	ImGui::Text("Indented Text");
		// 	ImGui::Unindent();
		// }

		// 3. 高级控件
		if (ImGui::CollapsingHeader("Advanced", EImGuiTreeNodeFlags::None))
		{
			// 颜色选择器
			// ImGui::ColorEdit4("Color Pick", ColorPick);

			// 下拉框
			TArray<FString> items;
            items.Add("Option 1");
            items.Add("Option 2");
            items.Add("Option 3");
			if (ImGui::BeginCombo("Combo", items[ComboSelection]))
			{
				for (int i = 0; i < items.Num(); i++)
				{
                    bool isSelected = ComboSelection == i;
					if (ImGui::Selectable(items[i],isSelected))
					{
						ComboSelection = i;
					}
				}
				ImGui::EndCombo();
			}

			// 进度条
			// ImGui::ProgressBar(Progress, FVector2f(100, 20));
			// Progress += 0.01f;
			// if (Progress > 1.0f)
			// 	Progress = 0.0f;

			// 树形视图
			// if (ImGui::TreeNode("Tree Node"))
			// {
			// 	if (ImGui::TreeNode("Child Node"))
			// 	{
			// 		ImGui::TreePop();
			// 	}
			// 	ImGui::TreePop();
			// }

			// 弹出窗口
			if (ImGui::Button("Show Popup"))
			{
				ImGui::OpenPopup("MyPopup");
			}
			if (ImGui::BeginPopup("MyPopup"))
			{
				ImGui::Text("Popup Content");
				if (ImGui::Button("Close"))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		// 4. 表格示例
		if (ImGui::CollapsingHeader("Table", EImGuiTreeNodeFlags::None))
		{
			if (ImGui::BeginTable("MyTable", 3, EImGuiTableFlags::Borders))
			{
				// 表头
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Value");
				ImGui::TableHeadersRow();

				// 数据行
				for (int row = 0; row < 3; row++)
				{
					ImGui::TableNextRow();
					// ImGui::TableSetColumnIndex(0);
					ImGui::Text("" + row);
					// ImGui::TableSetColumnIndex(1);
					ImGui::Text("Item " + row);
					// ImGui::TableSetColumnIndex(2);
					ImGui::Text("" + (row * 10));
				}
				ImGui::EndTable();
			}
		}

		ImGui::End();
	}
};