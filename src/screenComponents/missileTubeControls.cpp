#include "playerInfo.h"
#include "missileTubeControls.h"
#include "powerDamageIndicator.h"

GuiMissileTubeControls::GuiMissileTubeControls(GuiContainer* owner, string id)
: GuiAutoLayout(owner, id, LayoutVerticalBottomToTop), load_type(MW_None), manual_aim(false), missile_target_angle(0)
{
    setSize(GuiElement::GuiSizeMax, GuiElement::GuiSizeMax);
    
    for(int n=0; n<max_weapon_tubes; n++)
    {
        TubeRow row;
        row.layout = new GuiAutoLayout(this, id + "_ROW_" + string(n), LayoutHorizontalLeftToRight);
        row.layout->setSize(GuiElement::GuiSizeMax, 50);
        row.load_button = new GuiButton(row.layout, id + "_" + string(n) + "_LOAD_BUTTON", "Load", [this, n]() {
            if (!my_spaceship)
                return;
            if (my_spaceship->weapon_tube[n].isEmpty())
            {
                if (load_type != MW_None)
                {
                    my_spaceship->commandLoadTube(n, load_type);
                }
            }
            else
            {
                my_spaceship->commandUnloadTube(n);
            }
        });
        row.load_button->setSize(150, 50);
        row.fire_button = new GuiButton(row.layout, id + "_" + string(n) + "_FIRE_BUTTON", "Fire", [this, n]() {
            if (!my_spaceship)
                return;
            if (my_spaceship->weapon_tube[n].isLoaded())
            {
                float target_angle = missile_target_angle;
                if (!manual_aim)
                {
                    target_angle = my_spaceship->weapon_tube[n].calculateFiringSolution(my_spaceship->getTarget());
                    if (target_angle == std::numeric_limits<float>::infinity())
                        target_angle = my_spaceship->getRotation() + my_spaceship->weapon_tube[n].getDirection();
                }
                my_spaceship->commandFireTube(n, target_angle);
            }
        });
        row.fire_button->setSize(350, 50);
        (new GuiPowerDamageIndicator(row.fire_button, id + "_" + string(n) + "_PDI", SYS_MissileSystem, ACenterRight))->setSize(GuiElement::GuiSizeMax, GuiElement::GuiSizeMax);
        row.loading_bar = new GuiProgressbar(row.layout, id + "_" + string(n) + "_PROGRESS", 0, 1.0, 0);
        row.loading_bar->setColor(sf::Color(128, 128, 128))->setSize(350, 50);
        row.loading_label = new GuiLabel(row.loading_bar, id + "_" + string(n) + "_PROGRESS_LABEL", "Loading", 35);
        row.loading_label->setSize(GuiElement::GuiSizeMax, GuiElement::GuiSizeMax);
        
        rows.push_back(row);
    }
    
    
    for(int n=MW_Count-1; n>=0; n--)
    {
        load_type_rows[n].layout = new GuiAutoLayout(this, id + "_ROW_" + string(n), LayoutHorizontalLeftToRight);
        load_type_rows[n].layout->setSize(GuiElement::GuiSizeMax, 40);
        
        load_type_rows[n].button = new GuiToggleButton(load_type_rows[n].layout, id + "_MW_" + string(n), getMissileWeaponName(EMissileWeapons(n)), [this, n](bool value) {
            if (value)
                load_type = EMissileWeapons(n);
            else
                load_type = MW_None;
            for(int idx=0; idx<MW_Count; idx++)
                load_type_rows[idx].button->setValue(idx == load_type);
        });
        load_type_rows[n].button->setTextSize(28)->setSize(220, 40);
    }
    load_type_rows[MW_Homing].button->setIcon("gui/icons/weapon-homing.png");
    load_type_rows[MW_Mine].button->setIcon("gui/icons/weapon-mine.png");
    load_type_rows[MW_EMP].button->setIcon("gui/icons/weapon-emp.png");
    load_type_rows[MW_Nuke].button->setIcon("gui/icons/weapon-nuke.png");
}

void GuiMissileTubeControls::onDraw(sf::RenderTarget& window)
{
    if (!my_spaceship)
        return;
    for(int n=0; n<MW_Count; n++)
    {
        load_type_rows[n].button->setText(getMissileWeaponName(EMissileWeapons(n)) + " [" + string(my_spaceship->weapon_storage[n]) + "/" + string(my_spaceship->weapon_storage_max[n]) + "]");
        load_type_rows[n].layout->setVisible(my_spaceship->weapon_storage_max[n] > 0);
    }
    
    for(int n=0; n<my_spaceship->weapon_tube_count; n++)
    {
        WeaponTube& tube = my_spaceship->weapon_tube[n];
        rows[n].layout->show();
        if (tube.canOnlyLoad(MW_Mine))
            rows[n].fire_button->setIcon("gui/icons/weapon-mine", ACenterLeft);
        else
            rows[n].fire_button->setIcon("gui/icons/missile", ACenterLeft, tube.getDirection());
        if(tube.isEmpty())
        {
            rows[n].load_button->setEnable(tube.canLoad(load_type));
            rows[n].load_button->setText("Load");
            rows[n].fire_button->disable()->show();
            rows[n].fire_button->setText("Empty");
            rows[n].loading_bar->hide();
        }else if(tube.isLoaded())
        {
            rows[n].load_button->enable();
            rows[n].load_button->setText("Unload");
            rows[n].fire_button->enable()->show();
            rows[n].fire_button->setText(getMissileWeaponName(tube.getLoadType()));
            rows[n].loading_bar->hide();
        }else if(tube.isLoading())
        {
            rows[n].load_button->disable();
            rows[n].load_button->setText("Load");
            rows[n].fire_button->hide();
            rows[n].fire_button->setText(getMissileWeaponName(tube.getLoadType()));
            rows[n].loading_bar->show();
            rows[n].loading_bar->setValue(tube.getLoadProgress());
            rows[n].loading_label->setText("Loading");
        }else if(tube.isUnloading())
        {
            rows[n].load_button->disable();
            rows[n].load_button->setText("Unload");
            rows[n].fire_button->hide();
            rows[n].fire_button->setText(getMissileWeaponName(tube.getLoadType()));
            rows[n].loading_bar->show();
            rows[n].loading_bar->setValue(tube.getUnloadProgress());
            rows[n].loading_label->setText("Unloading");
        }else if(tube.isFiring())
        {
            rows[n].load_button->disable();
            rows[n].load_button->setText("Load");
            rows[n].fire_button->disable()->show();
            rows[n].fire_button->setText("Firing");
            rows[n].loading_bar->hide();
        }
    }
    for(int n=my_spaceship->weapon_tube_count; n<max_weapon_tubes; n++)
        rows[n].layout->hide();
    GuiAutoLayout::onDraw(window);
}

void GuiMissileTubeControls::setMissileTargetAngle(float angle)
{
    missile_target_angle = angle;
}

float GuiMissileTubeControls::getMissileTargetAngle()
{
    return missile_target_angle;
}

void GuiMissileTubeControls::setManualAim(bool manual)
{
    manual_aim = manual;
}

bool GuiMissileTubeControls::getManualAim()
{
    return manual_aim;
}
