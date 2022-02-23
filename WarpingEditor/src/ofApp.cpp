#include "ofApp.h"
#include "Models.h"
#include "GuiFunc.h"
#include "Icon.h"
#include "ImGuiFileDialog.h"

namespace {
template<typename T>
auto getJsonValue(const ofJson &json, const std::string &key, T default_value={}) -> T {
	return json.contains(key) ? json[key].get<T>() : default_value;
}
std::shared_ptr<ImageSource> buildTextureSource(const ProjectFolder &proj) {
	std::shared_ptr<ImageSource> ret = std::make_shared<ImageSource>();
	switch(proj.getTextureType()) {
		case ProjectFolder::Texture::FILE:
			if(ret->loadFromFile(proj.getTextureFilePath())) {
				return ret;
			}
			break;
		case ProjectFolder::Texture::NDI:
			if(ret->setupNDI(proj.getTextureNDIName())) {
				return ret;
			}
			break;
	}
	return nullptr;
}
}
//--------------------------------------------------------------
void GuiApp::setup(){
	ofDisableArbTex();
	Icon::init();
	
	ofEnableArbTex();
	ofBackground(0);

	gui_.setup(nullptr, true, ImGuiConfigFlags_DockingEnable, true);
	
	ndi_finder_.watchSources();
	
	uv_.setup();
	warp_.setup();
	
	editor_.push_back(&uv_);
	editor_.push_back(&warp_);
	
	loadRecent();
	openRecent();
}

//--------------------------------------------------------------
void GuiApp::update(){
	auto &data = Data::shared();
	if(texture_source_) {
		auto tex = texture_source_->getTexture();
		texture_source_->update();
		if(texture_source_->isFrameNew()) {
			main_app_->setTexture(tex);
			for(auto &&e : editor_) {
				e->setTexture(tex);
			}
		}
		if(tex.isAllocated()) {
			glm::vec2 tex_size{tex.getWidth(), tex.getHeight()};
			glm::vec2 tex_size_cache = proj_.getTextureSizeCache();
			if(tex_size_cache != tex_size) {
				data.uvRescale(tex_size/tex_size_cache);
				proj_.setTextureSizeCache(tex_size);
			}
		}
	}
	bool gui_focused = ImGui::GetIO().WantCaptureMouse;
	for(auto &&e : editor_) {
		e->setEnableViewportEditByMouse(!gui_focused);
		e->setEnableMeshEditByMouse(!gui_focused);
	}

	bool update_mesh = true;
	auto editor = editor_[state_];
	if(editor) {
		editor->setRegion(ofGetCurrentViewport());
		editor->update();
		update_mesh = !editor->isPreventMeshInterpolation();
	}

	if(update_mesh) {
		data.update();
		if(texture_source_) {
			auto tex = texture_source_->getTexture();
			if(tex.isAllocated()) {
				auto tex_data = tex.getTextureData();
				glm::vec2 tex_scale = tex_data.textureTarget == GL_TEXTURE_RECTANGLE_ARB
				? glm::vec2{1,1}
				: glm::vec2{1/tex_data.tex_w, 1/tex_data.tex_h};
				main_app_->setMesh(data.getMeshForExport(100, tex_scale));
			}
		}
	}
}

//--------------------------------------------------------------
void GuiApp::draw(){
	auto editor = editor_[state_];
	if(editor) {
		editor->draw();
	}
	
	gui_.begin();
	using namespace ImGui;
	Shortcut sc_save{[&]{save();}, 'S'};
	Shortcut sc_save_as{[&]{ImGuiFileDialog::Instance()->OpenModal("SaveProjDlgKey", "Save Project Folder", nullptr, ofToDataPath(""));}, 'S', Shortcut::MOD_FLAG_DEFAULT|ImGuiKeyModFlags_Shift};
	Shortcut sc_open{[&]{ImGuiFileDialog::Instance()->OpenModal("ChooseProjDlgKey", "Choose Project Folder", nullptr, ofToDataPath(""));}, 'O'};
	Shortcut sc_export{[&]{exportMesh(proj_);}, 'E'};
	bool is_open_export_dialog_trigger=false;
	Shortcut sc_export_to{[&]{is_open_export_dialog_trigger = true;}, 'E', Shortcut::MOD_FLAG_DEFAULT|ImGuiKeyModFlags_Shift};
	for(auto &&s : std::vector<Shortcut>{sc_save,sc_save_as,sc_open,sc_export,sc_export_to}) {
		if(s.check()) {
			s();
		}
	}
	/*
	auto &&io = ImGui::GetIO();
	std::cout << "mod1:" << (Shortcut::MOD_FLAG_DEFAULT|ImGuiKeyModFlags_Shift) << std::endl;
	std::cout << "mod2:" << io.KeyMods << std::endl;
	for(int i = 0; i < 512; ++i) {
		if(io.KeysDown[i]) {
			std::cout << "keys:" << i << std::endl;
		}
	}
	 */
	std::vector<std::string> shortcut_triggers;
	if(BeginMainMenuBar()) {
		if(BeginMenu("Project")) {
			if(MenuItem("Open file...", sc_open.keyStr().c_str())) {
				sc_open();
			}
			Separator();
			if(BeginMenu("recent")) {
				for(auto &&r : recent_) {
					std::stringstream ss;
					ss << r.getRelative() << "(" << r.getAbsolute() << ")";
					if(MenuItem(ss.str().c_str())) {
						openProject(r.getAbsolute());
					}
				}
				EndMenu();
			}
			Separator();
			if(MenuItem("Save", sc_save.keyStr().c_str())) {
				sc_save();
			}
			if(MenuItem("Save as...", sc_save_as.keyStr().c_str())) {
				sc_save_as();
			}
			EndMenu();
		}
		if(BeginMenu("Texture")) {
			if(BeginMenu("Image File")) {
				if(MenuItem("Load from file...")) {
					ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey", "Choose File", "{.png,.gif,.jpg,.jpeg,.mov,.mp4}", ofFilePath::addTrailingSlash(proj_.getAbsolute().string()));
				}
				Separator();
				std::string filepath;
				if(SelectFileMenu(proj_.getRelative().string(), filepath, true, {"png","gif","jpg","jpeg","mov","mp4"})) {
					filepath = ofToDataPath(filepath, true);
					proj_.setTextureSourceFile(filepath);
					if((texture_source_ = buildTextureSource(proj_))) {
						auto tex = texture_source_->getTexture();
						if(tex.isAllocated()) {
							main_app_->setTexture(tex);
							auto editor = editor_[state_];
							if(editor) {
								editor->setTexture(tex);
							}
						}
					}
				}
				EndMenu();
			}
			if(BeginMenu("NDI")) {
				auto source = ndi_finder_.getSources();
				for(auto &&s : source) {
					std::stringstream ss;
					ss << s.ndi_name << "(" << s.url_address << ")";
					if(MenuItem(ss.str().c_str())) {
						proj_.setTextureSourceNDI(s.ndi_name);
						texture_source_ = buildTextureSource(proj_);
					}
				}
				EndMenu();
			}
			EndMenu();
		}
		if(BeginMenu("Export")) {
			if(MenuItem("export by recent settings", sc_export.keyStr().c_str())) {
				sc_export();
			}
			if(MenuItem("export...", sc_export_to.keyStr().c_str())) {
				sc_export_to();
			}
			EndMenu();
		}
		EndMainMenuBar();
	}

	if(ImGuiFileDialog::Instance()->Display("SelectFolderDlgKey")) {
		if (ImGuiFileDialog::Instance()->IsOk() == true) {
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			proj_.setExportFolder(filePathName);
			is_open_export_dialog_trigger = true;
		}
		ImGuiFileDialog::Instance()->Close();
	}
	if(is_open_export_dialog_trigger) {
		OpenPopup("is_open_export_dialog_trigger");
	}
	if(BeginPopupModal("is_open_export_dialog_trigger")) {
		std::string folder = proj_.getExportFolder();
		float resample_min_interval = proj_.getExportMeshMinInterval();
		bool is_arb = proj_.getIsExportMeshArb();
		std::string filename = proj_.getExportFileName();
		if(InputFloat("resample_min_interval", &resample_min_interval)) {
			proj_.setExportMeshMinInterval(resample_min_interval);
		}
		if(Checkbox("arb", &is_arb)) {
			proj_.setIsExportMeshArb(is_arb);
		}
		if(EditText("export folder", folder, 1024)) {
			proj_.setExportFolder(folder);
		}
		SameLine();
		if(Button("...")) {
			ImGuiFileDialog::Instance()->OpenModal("SelectFolderDlgKey", "Choose Export Folder", nullptr, folder);
		}
		if(EditText("filename", filename)) {
			proj_.setExportFileName(filename);
		}
		if(Button("export")) {
			exportMesh(proj_);
			CloseCurrentPopup();
		} SameLine();
		if(Button("cancel")) {
			CloseCurrentPopup();
		}
		EndPopup();
	}
	if(ImGuiFileDialog::Instance()->Display("ChooseProjDlgKey")) {
		if (ImGuiFileDialog::Instance()->IsOk() == true) {
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			openProject(filePathName);
		}
		ImGuiFileDialog::Instance()->Close();
	}
	if(ImGuiFileDialog::Instance()->Display("SaveProjDlgKey")) {
		if (ImGuiFileDialog::Instance()->IsOk() == true) {
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			proj_.WorkFolder::setRelative(filePathName);
			save();
			proj_.setup();
			updateRecent(proj_);
		}
		ImGuiFileDialog::Instance()->Close();
	}
	if(ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
		if (ImGuiFileDialog::Instance()->IsOk() == true) {
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			proj_.setTextureSourceFile(filePathName);
			if((texture_source_ = buildTextureSource(proj_))) {
				auto tex = texture_source_->getTexture();
				if(tex.isAllocated()) {
					main_app_->setTexture(tex);
					auto editor = editor_[state_];
					if(editor) {
						editor->setTexture(tex);
					}
				}
			}
		}
		ImGuiFileDialog::Instance()->Close();
	}
	if(Begin("MainWindow")) {
		auto position = main_window_->getWindowPosition();
		if(DragFloat2("position", &position.x, 1, 0, -1, "%0.0f")) {
			main_window_->setWindowPosition(position.x, position.y);
		}
		auto size = main_window_->getWindowSize();
		if(DragFloat2("size", &size.x, 1, 1, -1, "%0.0f")) {
			main_window_->setWindowShape(size.x, size.y);
		}
	}
	End();
	if(Begin("MeshList")) {
		auto &data = Data::shared();
		static std::pair<std::string, std::weak_ptr<Data::Mesh>> mesh_edit;
		static std::string mesh_name_buf;
		static bool need_keyboard_focus=false;
		bool update_mesh_name = false;
		std::weak_ptr<Data::Mesh> mesh_delete;
		
		std::map<std::string, std::shared_ptr<Data::Mesh>> selected_meshes;
		auto &meshes = data.getMesh();
		for(auto &&m : meshes) {
			PushID(m.first.c_str());
			ToggleButton("##hide", m.second->is_hidden, Icon::HIDE, Icon::SHOW, {17,17}, 0);	SameLine();
			ToggleButton("##lock", m.second->is_locked, Icon::LOCK, Icon::UNLOCK, {17,17}, 0);	SameLine();
			ToggleButton("##solo", m.second->is_solo, Icon::FLAG, Icon::BLANK, {17,17}, 0);	SameLine();
			if(mesh_edit.second.lock() == m.second) {
				if(need_keyboard_focus) SetKeyboardFocusHere();
				need_keyboard_focus = false;
				update_mesh_name = EditText("###change name", mesh_name_buf, 256, ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_AutoSelectAll);
			}
			else {
				bool selected
				= editor == &uv_ ? selected = uv_.isSelectedMesh(*m.second)
				: editor == &warp_ ? selected = warp_.isSelectedMesh(*m.second)
				: false;
				if(Selectable(m.first.c_str(), &selected)) {
					editor == &uv_ ? selected ? uv_.selectMesh(*m.second) : uv_.deselectMesh(*m.second)
					: editor == &warp_ ? selected ? warp_.selectMesh(*m.second) : warp_.deselectMesh(*m.second)
					: false;
				}
				if(selected) {
					selected_meshes.insert(m);
				}
				if(IsItemClicked(ImGuiPopupFlags_MouseButtonLeft)) {
					mesh_edit.second.reset();
				}
				else if(IsItemClicked(ImGuiPopupFlags_MouseButtonMiddle)
						|| (IsItemClicked(ImGuiPopupFlags_MouseButtonRight) && IsModKeyDown(ImGuiKeyModFlags_Alt))
					) {
					mesh_delete = m.second;
				}
				else if(IsItemClicked(ImGuiPopupFlags_MouseButtonRight)) {
					mesh_name_buf = m.first;
					mesh_edit = m;
					need_keyboard_focus = true;
				}
			}
			PopID();
		}
		if(update_mesh_name) {
			auto found = meshes.find(mesh_edit.first);
			assert(found != end(meshes));
			if(mesh_name_buf != "" && meshes.insert({mesh_name_buf, found->second}).second) {
				meshes.erase(found);
			}
			mesh_edit.second.reset();
		}
		if(auto mesh_to_delete = mesh_delete.lock()) {
			data.remove(mesh_to_delete);
		}
		if(Button("create new")) {
			auto tex = texture_source_->getTexture();
			if(tex.isAllocated()) {
				data.create("mesh", {0,0,tex.getWidth(),tex.getHeight()});
			}
		}
		if(mesh_edit.second.expired() && !selected_meshes.empty()) {
			SameLine();
			if(Button("duplicate selected")) {
				for(auto &&s : selected_meshes) {
					data.createCopy(s.first, s.second);
				}
			}
		}
	}
	End();
	if(editor) {
		editor->gui();
	}
}

void GuiApp::exportMesh(float resample_min_interval, const std::filesystem::path &filepath, bool is_arb) const
{
	auto tex = texture_source_->getTexture();
	glm::vec2 coord_size = is_arb&&tex.isAllocated()?glm::vec2{1,1}: glm::vec2{1/tex.getWidth(), 1/tex.getHeight()};
	Data::shared().exportMesh(filepath, resample_min_interval, coord_size);
}
void GuiApp::exportMesh(const ProjectFolder &proj) const
{
	std::string folder = proj_.getExportFolder();
	float resample_min_interval = proj_.getExportMeshMinInterval();
	bool is_arb = proj_.getIsExportMeshArb();
	std::string filename = proj_.getExportFileName();
	exportMesh(resample_min_interval, ofFilePath::join(folder, filename), is_arb);
}


//--------------------------------------------------------------
void GuiApp::keyPressed(int key){
	if(ImGui::GetIO().WantCaptureKeyboard) {
		return;
	}
	glm::vec2 move{0,0};
	switch(key) {
		case OF_KEY_TAB:
			state_ ^= 1;
			break;
		case OF_KEY_LEFT: move.x = -1; break;
		case OF_KEY_RIGHT: move.x = 1; break;
		case OF_KEY_UP: move.y = -1; break;
		case OF_KEY_DOWN: move.y = 1; break;
	}
	if(ImGui::IsModKeyDown(ImGuiKeyModFlags_Shift)) {
		move *= 10;
	}
	auto editor = editor_[state_];
	if(editor) {
		editor->moveSelectedOnScreenScale(move);
	}
}

void GuiApp::save(bool do_backup) const
{
	{
		auto pos = main_window_->getWindowPosition();
		auto size = main_window_->getWindowSize();
		proj_.setMainViewport({pos.x,pos.y,size.x,size.y});
	}
	proj_.setUVView(-uv_.getTranslate(), uv_.getScale());
	proj_.setUVGridData(uv_.getGridData());
	proj_.setWarpView(-warp_.getTranslate(), warp_.getScale());
	proj_.setWarpGridData(warp_.getGridData());
	proj_.save();

	auto filepath = proj_.getDataFilePath();
	auto tex_size = proj_.getTextureSizeCache();
	Data::shared().save(filepath, {1/tex_size.x, 1/tex_size.y});
	
	if(do_backup && proj_.isBackupEnabled()) {
		auto backup_path = proj_.getBackupFilePath();
		ofDirectory folder(ofFilePath::getEnclosingDirectory(backup_path));
		if(!folder.exists()) {
			folder.create();
		}
		ofFile(filepath).copyTo(backup_path);
		int num = proj_.getBackupNumLimit();
		if(num > 0) {
			folder.allowExt(ofFilePath::getFileExt(backup_path));
			folder.sortByDate();
			for(int i = 0; i < folder.size() - num; ++i) {
				folder.getFile(i).remove();
			}
		}
	}
}

void GuiApp::openProject(const std::filesystem::path &proj_path)
{
	proj_.WorkFolder::setRelative(proj_path);
	proj_.setup();

	{
		auto view = proj_.getMainViewport();
		main_window_->setWindowPosition(view[0], view[1]);
		main_window_->setWindowShape(view[2], view[3]);
	}
	{
		auto view = proj_.getUVView();
		uv_.resetMatrix();
		uv_.translate(view.first);
		uv_.scale(view.second, {0,0});
		uv_.setGridData(proj_.getUVGridData());
	}
	{
		auto view = proj_.getWarpView();
		warp_.resetMatrix();
		warp_.translate(view.first);
		warp_.scale(view.second, {0,0});
		warp_.setGridData(proj_.getWarpGridData());
	}
	updateRecent(proj_);

	if((texture_source_ = buildTextureSource(proj_))) {
		auto tex = texture_source_->getTexture();
		if(tex.isAllocated()) {
			main_app_->setTexture(tex);
			for(auto &&e : editor_) {
				e->setTexture(tex);
			}
		}
	}
	Data::shared().load(proj_.getDataFilePath(), proj_.getTextureSizeCache());
}

void GuiApp::openRecent(int index)
{
	auto json = ofLoadJson("project_folder.json");
	auto most_recent = getJsonValue<std::vector<ofJson>>(json, "recent");
	if(index >= 0 && most_recent.size() > index) {
		auto proj_path = getJsonValue<std::string>(most_recent[index], "abs");
		openProject(proj_path);
	}
}

void GuiApp::loadRecent()
{
	auto recent = getJsonValue<ofJson>(ofLoadJson("project_folder.json"), "recent");
	recent_ = accumulate(begin(recent), end(recent), decltype(recent_){}, [](decltype(recent_) acc, const ofJson &json) {
		WorkFolder w;
		w.setAbsolute(json["abs"].get<std::string>());
		acc.push_back(w);
		return acc;
	});
}
void GuiApp::updateRecent(const ProjectFolder &proj)
{
	auto found = find_if(begin(recent_), end(recent_), [proj](const WorkFolder &w) {
		return w.toJson() == proj.WorkFolder::toJson();
	});
	if(found != end(recent_)) {
		recent_.erase(found);
	}
	recent_.push_front(proj);
	auto recent = accumulate(begin(recent_), end(recent_), std::vector<ofJson>{}, [](std::vector<ofJson> acc, const WorkFolder &w) {
		acc.push_back(w.toJson());
		return acc;
	});
	ofSavePrettyJson("project_folder.json", {{"recent", recent}});
}

void GuiApp::dragEvent(ofDragInfo dragInfo)
{
	if(dragInfo.files.empty()) return;
	auto filepath = dragInfo.files[0];
	auto ext = ofFilePath::getFileExt(filepath);
	if(ext == "maap") {
		Data::shared().load(filepath, proj_.getTextureSizeCache());
	}
}

//--------------------------------------------------------------
void MainApp::setup()
{
	ofBackground(0);
}
void MainApp::update()
{
}
void MainApp::draw()
{
	texture_.bind();
	mesh_.draw();
	texture_.unbind();
}
