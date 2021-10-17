#include <vector>
#include "headers/classes.h"
#include "headers/draw.h"
#include "headers/generate.h"
#include "headers/mode.h"
#include "headers/main.h"
bool check_surroundings(std::vector<monster> monsters, int x, int y){
    for(int i=0; i<monsters.size(); i++){
        if(x==monsters[i].x&&y==monsters[i].y){
            return true;
        }
    }
    return false;
}

std::pair<int, Dungeon> check_monster_pos(std::vector<monster> monsters, int x, int y){
    for(int i=0; i<monsters.size(); i++){
        if(x==monsters[i].x&&y==monsters[i].y){
            return {i, monsters[i].type};
        }
    }
    return {-1, Dungeon::NONE};
}

void char_move(tcod::ConsolePtr &main_win, tcod::ContextPtr &context, int ch, Csr &csr_pos, const std::vector<monster> &monsters, Player &User, NPC &npc, level Current){
    bool require_move=false;
    if((ch=='a'||ch==SDLK_LEFT)&&csr_pos.first>1&&!check_surroundings(monsters, csr_pos.first-1, csr_pos.second)){
        TCOD_console_put_char_ex(main_win.get(), csr_pos.first, csr_pos.second+1, ' ', WHITE, BLACK);
        csr_pos.first--;
        require_move=true;
    }
    else if((ch=='d'||ch==SDLK_RIGHT)&&csr_pos.first<78&&!check_surroundings(monsters, csr_pos.first+1, csr_pos.second)){
        TCOD_console_put_char_ex(main_win.get(), csr_pos.first, csr_pos.second+1, ' ', WHITE, BLACK);
        csr_pos.first++;
        require_move=true;
    }
    else if((ch=='w'||ch==SDLK_UP)&&csr_pos.second>1&&!check_surroundings(monsters, csr_pos.first, csr_pos.second-1)){
        TCOD_console_put_char_ex(main_win.get(), csr_pos.first, csr_pos.second+1, ' ', WHITE, BLACK);
        csr_pos.second--;
        require_move=true;
    }
    else if((ch=='s'||ch==SDLK_DOWN)&&csr_pos.second<48&&!check_surroundings(monsters, csr_pos.first, csr_pos.second+1)){
        TCOD_console_put_char_ex(main_win.get(), csr_pos.first, csr_pos.second+1, ' ', WHITE, BLACK);
        csr_pos.second++;
        require_move=true;
    }
    if(require_move){
        User.steps++;
        if(User.steps%25==0){
            User.hydration--;
        }
        if(User.steps%35==0){
            User.saturation--;
        }
        if(User.steps==npc.bank.interest_next_applied){
            npc.bank.interest_next_applied+=50000;
            npc.bank.saved_gold*=npc.bank.storage_interest;
        }
        draw_doors(main_win, context, Current);
        draw_player(main_win, context, csr_pos.first, csr_pos.second);
        draw_stats(main_win, context, User);
        context->present(*main_win);
    }
}

void process_gear(Player &User, Item *&processed_item, int damage){
    if(processed_item!=nullptr){
        processed_item->uses++;
        processed_item->durability-=((damage/500.0)*(1+((101-processed_item->durability)/100)))/rarity_value(processed_item->rarity);
        if(processed_item->durability<=0.0){
            processed_item->durability=0;
            processed_item->is_equipped=false;
            processed_item=nullptr;
        }
        else{
            processed_item->reinitialize_item();
        }
    }
}

std::pair<std::string, std::string> calculate_damage(Player &User, monster_stats &monster){
    std::stringstream first, second;
    if(User.crit_chance>0&&return_chance(User.crit_chance)){ // Calculate damage that enemy takes
        monster.hp=monster.hp-(User.attk*(1+(User.crit_dmg/100.0)));
        first << "Critical Hit: " << "You dealt " << User.attk*(1+(User.crit_dmg/100.0)) << " damage";
    }
    else{
        monster.hp=monster.hp-(User.attk-monster.def);
        first << "You dealt " << User.attk-monster.def << " damage";
    }
    if(User.def<monster.attk){ // Calculate damage I take
        if(User.cur_shield>monster.attk){
            User.cur_shield-=monster.attk;
            second << "You blocked " << monster.attk << " damage";
        }
        else if(User.cur_shield<monster.attk&&User.cur_shield>0){
            User.cur_hp-=(monster.attk-User.cur_shield);
            second << "You received " << monster.attk-User.cur_shield << " damage";
            User.cur_shield=0;
        }
        else{
            User.cur_hp-=(monster.attk-User.def);
            second << "You received " << monster.attk-User.def << " damage";
        }
    }
    else{
        second << "You received no damage";
    }
    User.uninitialize_stats();
    process_gear(User, User.equip.helmet, monster.attk);
    process_gear(User, User.equip.chestplate, monster.attk);
    process_gear(User, User.equip.greaves, monster.attk);
    process_gear(User, User.equip.boots, monster.attk);
    process_gear(User, User.equip.shield, monster.attk);
    process_gear(User, User.equip.weapon, monster.attk);
    User.initialize_stats();
    return {first.str(), second.str()};
}

bool player_battle(tcod::ConsolePtr &main_win, tcod::ContextPtr &context, Player &User, level Current, Dungeon monster_type){
    monster_stats monster=create_monster(Current, monster_type);
    std::pair<std::string, std::string> log;
    int ch;
    while(true){
        SDL_wclear_main_win(main_win, context);
        draw_border(main_win, context);
        std::stringstream user_output;
        std::stringstream enemy_output;
        draw_stats(main_win, context, User);
        user_output << username << "~ HP:" << User.cur_hp << " Defence:" << User.def << " Shield:" << User.cur_shield << " Heal:" << User.inv.misc.heal_amount;
        enemy_output << "Enemy~ HP:" << monster.hp << " Attk:" << monster.attk << " Def:" << monster.def;
        tcod::print(*main_win, {1, 49}, user_output.str(), &WHITE, &BLACK, TCOD_BKGND_SET, TCOD_LEFT);
        tcod::print(*main_win, {1, 2}, enemy_output.str(), &WHITE, &BLACK, TCOD_BKGND_SET, TCOD_LEFT);
        tcod::print(*main_win, {39, 25}, log.first, &WHITE, &BLACK, TCOD_BKGND_SET, TCOD_CENTER);
        tcod::print(*main_win, {39, 26}, log.second, &WHITE, &BLACK, TCOD_BKGND_SET, TCOD_CENTER);
        context->present(*main_win);
        ch=SDL_getch(main_win, context);
        if(ch=='1'){
            log=calculate_damage(User, monster);
            if(User.cur_hp<=0){
                return false;
            }
            if(monster.hp<=0){
                User.gold+=generate_gold(monster_type);
                Item loot=generate_loot_from_monster_type(monster_type);
                SDL_wclear_main_win(main_win, context);
                tcod::print(*main_win, {0, 1}, "Press any key to keep and [r] to trash", &WHITE, &BLACK, TCOD_BKGND_SET, TCOD_LEFT);
                draw_stats(main_win, context, User);
                print_description(main_win, context, &loot, 1);

                ch=SDL_getch(main_win, context);
                while(!(ch>0&&ch<128)){
                    ch=SDL_getch(main_win, context);
                }
                if(ch=='r'){
                    return true;
                }
                User.add_item(loot);
                return true;
            }
        }
        else if(ch=='2'){
            if(User.inv.misc.heal_amount>0&&User.cur_hp!=User.ori_hp){
                User.inv.misc.heal_amount--;
                User.cur_hp=User.ori_hp;
            }
        }
    }
}

std::pair<bool, bool> attack_monster(tcod::ConsolePtr &main_win, tcod::ContextPtr &context, std::vector<monster> &monsters, Csr csr_pos, Player &User, level Current){
    std::pair<bool, bool> attack_status{false, false}; // .first = if surroundings have monsters, .second is_alive
    std::pair<int, Dungeon> pos;
    if(check_surroundings(monsters, csr_pos.first-1, csr_pos.second)){
        attack_status.first=true;
        pos=check_monster_pos(monsters, csr_pos.first-1, csr_pos.second);
        attack_status.second=player_battle(main_win, context, User, Current, pos.second);
        monsters.erase(monsters.begin()+pos.first);
    }
    else if(check_surroundings(monsters, csr_pos.first+1, csr_pos.second)){
        attack_status.first=true;
        pos=check_monster_pos(monsters, csr_pos.first+1, csr_pos.second);
        attack_status.second=player_battle(main_win, context, User, Current, pos.second);
        monsters.erase(monsters.begin()+pos.first);
    }
    else if(check_surroundings(monsters, csr_pos.first, csr_pos.second-1)){
        attack_status.first=true;
        pos=check_monster_pos(monsters, csr_pos.first, csr_pos.second-1);
        attack_status.second=player_battle(main_win, context, User, Current, pos.second);
        monsters.erase(monsters.begin()+pos.first);
    }
    else if(check_surroundings(monsters, csr_pos.first, csr_pos.second+1)){
        attack_status.first=true;
        pos=check_monster_pos(monsters, csr_pos.first, csr_pos.second+1);
        attack_status.second=player_battle(main_win, context, User, Current, pos.second);
        monsters.erase(monsters.begin()+pos.first);
    }
    return {attack_status};
}

void move_door(std::vector<std::pair<int, int>> &doors, std::vector<monster> &monsters, level &Current, Csr &csr_pos){
    for(int i=0; i<doors.size(); i++){
        if(doors[i].first==csr_pos.first&&doors[i].second==csr_pos.second-1){
            ++Current.y;
            csr_pos={39, 48};
            doors.clear();
            monsters.clear();
            generate_doors(doors, Current);
            generate_monsters(monsters, Current, csr_pos);
            break;
        }
        else if(doors[i].first==csr_pos.first&&doors[i].second==csr_pos.second+1){
            --Current.y;
            csr_pos={39, 1};
            doors.clear();
            monsters.clear();
            generate_doors(doors, Current);
            generate_monsters(monsters, Current, csr_pos);
            break;
        }
        else if(doors[i].first==csr_pos.first+1&&doors[i].second==csr_pos.second){
            ++Current.x;
            csr_pos={1, 24};
            doors.clear();
            monsters.clear();
            generate_doors(doors, Current);
            generate_monsters(monsters, Current, csr_pos);
            break;
        }
        else if(doors[i].first==csr_pos.first-1&&doors[i].second==csr_pos.second){
            --Current.x;
            csr_pos={78, 24};
            doors.clear();
            monsters.clear();
            generate_doors(doors, Current);
            generate_monsters(monsters, Current, csr_pos);
            break;
        }
    }
}

bool move_staircase(level &Current, Csr csr_pos){
    if(Current.lvl<5&&Current.x==5&&Current.y==5){ // go down
        if(csr_pos.first==39&&csr_pos.second==24){
            Current.lvl++;
            Current.reset(0);
            return true;
        }
    }
    if(Current.lvl>1&&Current.x==1&&Current.y==1){ // go down
        if(csr_pos.first==39&&csr_pos.second==24){
            Current.lvl--;
            Current.reset(1);
            return true;
        }
    }
    return false;
}

void print_food(tcod::ConsolePtr &main_win, tcod::ContextPtr &context, Player &User){
    SDL_wclear_main_win(main_win, context);
    std::stringstream ss;
    ss << "[1] Bread (30 Saturation Points): " << User.inv.food.bread;
    tcod::print(*main_win, {0, 1}, ss.str(), &WHITE, &BLACK, TCOD_BKGND_SET, TCOD_LEFT);
    ss.str(std::string());
    ss << "[2] Waffle (50 Saturation Points): " << User.inv.food.waffle;
    tcod::print(*main_win, {0, 2}, ss.str(), &WHITE, &BLACK, TCOD_BKGND_SET, TCOD_LEFT);
    ss.str(std::string());
    ss << "[3] Energy Bar (100 Saturation Points): " << User.inv.food.energy_bar;
    tcod::print(*main_win, {0, 3}, ss.str(), &WHITE, &BLACK, TCOD_BKGND_SET, TCOD_LEFT);
    ss.str(std::string());
    ss << "[4] Water (50 Hydration Points): " << User.inv.water.water;
    tcod::print(*main_win, {0, 4}, ss.str(), &WHITE, &BLACK, TCOD_BKGND_SET, TCOD_LEFT);
    ss.str(std::string());
    ss << "[5] Sparkling Water (100 Hydration Points): " << User.inv.water.sparkling_juice;
    tcod::print(*main_win, {0, 5}, ss.str(), &WHITE, &BLACK, TCOD_BKGND_SET, TCOD_LEFT);
    ss.str(std::string());
    context->present(*main_win);
}

void eat_drink_mode(tcod::ConsolePtr &main_win, tcod::ContextPtr &context, Player &User){
    int ch;
    while(true){
        print_food(main_win, context, User);
        draw_stats(main_win, context, User);
        ch=SDL_getch(main_win, context);
        if(ch=='1'&&User.inv.food.bread>0){
            User.eat(&User.inv.food.bread);
        }
        else if(ch=='2'&&User.inv.food.waffle>0){
            User.eat(&User.inv.food.waffle);
        }
        else if(ch=='3'&&User.inv.food.energy_bar>0){
            User.eat(&User.inv.food.energy_bar);
        }
        else if(ch=='4'&&User.inv.water.water>0){
            User.drink(&User.inv.water.water);
        }
        else if(ch=='5'&&User.inv.water.sparkling_juice>0){
            User.drink(&User.inv.water.sparkling_juice);
        }
        else if(ch=='q'){
            return;
        }
    }
}

void drop_items_on_death(Player &User, Csr &csr_pos, level &current){
    User=Player();
    csr_pos={1, 1};
    current={1, 1, 1};
}

void init_dungeon(tcod::ConsolePtr &main_win, tcod::ContextPtr &context, Csr &csr_pos, Player &User, level &Current, std::vector<monster> &monsters, NPC &npc, NoDelete &perm_config){
    std::vector<std::pair<int, int>> doors;
    if(monsters.empty()){
        generate_monsters(monsters, Current, csr_pos);
    }
    generate_doors(doors, Current);
    redraw_everything(main_win, context, csr_pos, User, Current, monsters);
    int ch;
    while(true){
        ch=SDL_getch(main_win, context);
        if(ch=='w'||ch=='a'||ch=='s'||ch=='d'||ch==SDLK_LEFT||ch==SDLK_RIGHT||ch==SDLK_DOWN||ch==SDLK_UP){
            char_move(main_win, context, ch, csr_pos, monsters, User, npc, Current);
        }
        else if(ch=='z'){
            move_door(doors, monsters, Current, csr_pos);
            redraw_everything(main_win, context, csr_pos, User, Current, monsters);
        }
        else if(ch=='x'){
            std::pair<bool, bool> attack_status=attack_monster(main_win, context, monsters, csr_pos, User, Current);
            if(attack_status.first&&!attack_status.second){ // If dead return to main menu
                end_program(1);
                drop_items_on_death(User, csr_pos, Current);
                save_data(User, Current, csr_pos, monsters, npc, perm_config);
                return;
            }
            else if(attack_status.first&&attack_status.second){ // If win redraw dungeon and move on
                User.cur_shield=User.ori_shield;
                redraw_everything(main_win, context, csr_pos, User, Current, monsters);
            }
        }
        else if(ch=='c'){
            if(move_staircase(Current, csr_pos)){
                generate_doors(doors, Current);
            }
            else if(Current.lvl==1&&Current.x==1&&Current.y==1&&csr_pos.first==1&&csr_pos.second==48){
                bar_mode(main_win, context, User, npc, perm_config);
            }
            redraw_everything(main_win, context, csr_pos, User, Current, monsters);
        }
        else if(ch=='i'){
            if(!User.inv.item.empty()){
                inventory_mode(main_win, context, User, perm_config);
                redraw_everything(main_win, context, csr_pos, User, Current, monsters);
            }
            else{
                clear_and_draw_dialog(main_win, context, "[System] Error: Inventory Empty");
            }
        }
        else if(ch=='e'){
            eat_drink_mode(main_win, context, User);
            redraw_everything(main_win, context, csr_pos, User, Current, monsters);
        }
        else if(ch=='.'){
            if(User.cur_hp<User.ori_hp){
                User.saturation-=1;
                User.cur_hp+=(User.ori_hp*0.015);
            }
            if(User.cur_hp>User.ori_hp){
                User.cur_hp=User.ori_hp;
            }
            redraw_everything(main_win, context, csr_pos, User, Current, monsters);
        }
        else if(ch=='S'){
            save_data(User, Current, csr_pos, monsters, npc, perm_config);
            redraw_everything(main_win, context, csr_pos, User, Current, monsters);
            clear_and_draw_dialog(main_win, context, "Data saved successfully!");
        }
        else if(ch=='H'){
            help_mode(main_win, context, "dungeon_mode");
            redraw_everything(main_win, context, csr_pos, User, Current, monsters);
        }
        else if(ch=='q'){
            if(SDL_getch_y_or_n(main_win, context, "Do you really wish to quit? [y] to quit, any other key to abort.")){
                end_program(0);
                save_data(User, Current, csr_pos, monsters, npc, perm_config);
                return;
            }
            redraw_everything(main_win, context, csr_pos, User, Current, monsters);
        }
        if(User.saturation<=0){
            drop_items_on_death(User, csr_pos, Current);
            save_data(User, Current, csr_pos, monsters, npc, perm_config);
            end_program(2);
            return;
        }
        if(User.hydration<=0){
            drop_items_on_death(User, csr_pos, Current);
            save_data(User, Current, csr_pos, monsters, npc, perm_config);
            end_program(3);
            return;
        }
    }
}

