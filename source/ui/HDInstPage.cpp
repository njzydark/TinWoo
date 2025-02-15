#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/HDInstPage.hpp"
#include "HDInstall.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;
    s32 zzz=0; //touchscreen variable

    HDInstPage::HDInstPage() : Layout::Layout() {
        this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#00000080"));
        this->SetBackgroundColor(COLOR("#000000FF"));
        this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#000000FF"));
        this->botRect = Rectangle::New(0, 659, 1280, 61, COLOR("#000000FF"));
        
    		if (inst::config::gayMode) {
    			if
    				(std::filesystem::exists(inst::config::appDir + "/images/Hd.png")) this->titleImage = Image::New(0, 0, (inst::config::appDir + "/images/Hd.png"));
    			else 
    				this->titleImage = Image::New(0, 0, "romfs:/images/Hd.png");
    			
    			if
    				(std::filesystem::exists(inst::config::appDir + "/images/Background.png")) this->SetBackgroundImage(inst::config::appDir + "/images/Background.png");
    			else
    				this->SetBackgroundImage("romfs:/images/Background.png");
            }
        
        else {
        			this->SetBackgroundImage("romfs:/images/Background.png");
        			this->titleImage = Image::New(0, 0, "romfs:/images/Hd.png");
              }
        this->pageInfoText = TextBlock::New(10, 109, "inst.hd.top_info"_lang);
        this->pageInfoText->SetFont(pu::ui::MakeDefaultFontName(30));
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->butText = TextBlock::New(10, 678, "inst.hd.buttons"_lang);
        this->butText->SetColor(COLOR("#FFFFFFFF"));
        this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR("#FFFFFF00"), COLOR("#4f4f4d33"), 84, 6);
        this->menu->SetScrollbarColor(COLOR("#1A1919FF"));
        this->Add(this->topRect);
        this->Add(this->infoRect);
        this->Add(this->botRect);
        this->Add(this->titleImage);
        this->Add(this->butText);
        this->Add(this->pageInfoText);
        this->Add(this->menu);
    }

    void HDInstPage::drawMenuItems(bool clearItems, std::filesystem::path ourPath) {
    	int myindex = this->menu->GetSelectedIndex(); //store index so when page redraws we can get the last item we checked.
        if (clearItems) this->selectedTitles = {};
		this->currentDir = ourPath;

		auto pathStr = this->currentDir.string();
		if(pathStr.length())
		{
			if(pathStr[pathStr.length() - 1] == ':')
			{
				this->currentDir = this->currentDir / "";
			}
		}

        this->menu->ClearItems();
        try {
            this->ourDirectories = util::getDirsAtPath(this->currentDir);
            this->ourFiles = util::getDirectoryFiles(this->currentDir, {".nsp", ".nsz", ".xci", ".xcz"});
        } catch (std::exception& e) {
            this->drawMenuItems(false, this->currentDir.parent_path());
            return;
        }

        std::string itm = "..";
        auto ourEntry = pu::ui::elm::MenuItem::New(itm);
        ourEntry->SetColor(COLOR("#FFFFFFFF"));
        ourEntry->SetIcon("romfs:/images/icons/folder-upload.png");
        this->menu->AddItem(ourEntry);

        for (auto& file: this->ourDirectories) {
            if (file == "..") break;
            std::string itm = file.filename().string();
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(COLOR("#FFFFFFFF"));
            ourEntry->SetIcon("romfs:/images/icons/folder.png");
            this->menu->AddItem(ourEntry);
        }
        for (auto& file: this->ourFiles) {
            std::string itm = file.filename().string();
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(COLOR("#FFFFFFFF"));
            ourEntry->SetIcon("romfs:/images/icons/checkbox-blank-outline.png");
            for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
                if (this->selectedTitles[i] == file) {
                    ourEntry->SetIcon("romfs:/images/icons/check-box-outline.png");
                }
            }
            this->menu->AddItem(ourEntry);
            this->menu->SetSelectedIndex(myindex); //jump to the index we saved from above
        }
    }

    void HDInstPage::followDirectory() {
        int selectedIndex = this->menu->GetSelectedIndex();
        int dirListSize = this->ourDirectories.size();

        dirListSize++;
        selectedIndex--;

        if (selectedIndex < dirListSize) {
            if (this->menu->GetItems()[this->menu->GetSelectedIndex()]->GetName() == ".." && this->menu->GetSelectedIndex() == 0) {
                this->drawMenuItems(true, this->currentDir.parent_path());
            } else {
                this->drawMenuItems(true, this->ourDirectories[selectedIndex]);
            }
            this->menu->SetSelectedIndex(0);
        }
    }

    void HDInstPage::selectNsp(int selectedIndex) {
        int dirListSize = this->ourDirectories.size();
        dirListSize++;

        if (this->menu->GetItems()[selectedIndex]->GetIconPath() == "romfs:/images/icons/check-box-outline.png") {
            for (long unsigned int i = 0; i < this->selectedTitles.size(); i++) {
                if (this->selectedTitles[i] == this->ourFiles[selectedIndex - dirListSize]) this->selectedTitles.erase(this->selectedTitles.begin() + i);
            }
        } else if (this->menu->GetItems()[selectedIndex]->GetIconPath() == "romfs:/images/icons/checkbox-blank-outline.png") this->selectedTitles.push_back(this->ourFiles[selectedIndex - dirListSize]);
        else {
            this->followDirectory();
            return;
        }
        this->drawMenuItems(false, currentDir);
    }

    void HDInstPage::startInstall() {
        int dialogResult = -1;
        if (this->selectedTitles.size() == 1) {
            dialogResult = mainApp->CreateShowDialog("inst.target.desc0"_lang + inst::util::shortenString(std::filesystem::path(this->selectedTitles[0]).filename().string(), 32, true) + "inst.target.desc1"_lang, "common.cancel_desc"_lang, {"inst.target.opt0"_lang, "inst.target.opt1"_lang}, false);
        } else dialogResult = mainApp->CreateShowDialog("inst.target.desc00"_lang + std::to_string(this->selectedTitles.size()) + "inst.target.desc01"_lang, "common.cancel_desc"_lang, {"inst.target.opt0"_lang, "inst.target.opt1"_lang}, false);
        if (dialogResult == -1) return;
        	nspInstStuff_B::installNspFromFile(this->selectedTitles, dialogResult);
    }

    void HDInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::TouchPoint touch_pos) {
        if (Down & HidNpadButton_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
         
        HidTouchScreenState state={0};
        
        if  (hidGetTouchScreenStates(&state, 1)) {
          
          if ((Down & HidNpadButton_A) || (state.count != zzz))
          {
              zzz = state.count;
              
              if (zzz != 1) {
              	this->selectNsp(this->menu->GetSelectedIndex());
              	if (this->ourFiles.size() == 1 && this->selectedTitles.size() == 1) {
              		this->startInstall();
              	}
              	
              }
          }
        }     
        
        if ((Down & HidNpadButton_Y)) {
            if (this->selectedTitles.size() == this->ourFiles.size()) this->drawMenuItems(true, currentDir);
            else {
                int topDir = 0;
                topDir++;
                for (long unsigned int i = this->ourDirectories.size() + topDir; i < this->menu->GetItems().size(); i++) {
                    if (this->menu->GetItems()[i]->GetIconPath() == "romfs:/images/icons/check-box-outline.png") continue;
                    else this->selectNsp(i);
                }
                this->drawMenuItems(false, currentDir);
            }
        }
        
        if ((Down & HidNpadButton_X)) {
            inst::ui::mainApp->CreateShowDialog("inst.hd.help.title"_lang, "inst.hd.help.desc"_lang, {"common.ok"_lang}, true);
        }
        
        if (Down & HidNpadButton_Plus) {
            if (this->selectedTitles.size() == 0 && this->menu->GetItems()[this->menu->GetSelectedIndex()]->GetIconPath() == "romfs:/images/icons/checkbox-blank-outline.png") {
                this->selectNsp(this->menu->GetSelectedIndex());
            }
            if (this->selectedTitles.size() > 0) this->startInstall();
        }
    }
}
