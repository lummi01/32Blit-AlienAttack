// AlienAttack
// 2024 M. Gerloff

#include <list>
#include "aa.hpp"
#include "assets.hpp"

using namespace blit;

Font font(font3x5);

struct GAME
{
    short state;
    int score;
    short level;
    short alien;
    short alien_timer;
};

struct PLAYER
{
    Vec2 pos;
    Vec2 pos0;
    short sprite;
    short ani;
    short shot;
};

struct SHOT
{
    Vec2 pos;
    Vec2 vel;
};

struct ALIEN
{
    short sprite;
    short ani;
    int alpha;
    Vec2 pos;
    Vec2 vel;
};

struct EXPLOSION
{
	Vec2 pos;
	int alpha;
    short sprite;
};

GAME game;
PLAYER p;

Timer ani_timer;

static std::list<SHOT> shot;
static std::list<ALIEN> alien;
static std::list<EXPLOSION> particles;

void NewExplosion(Vec2 pos, short sprite)
{
    EXPLOSION e;
    e.sprite = sprite;
    e.pos = pos;
    e.alpha = 255;
    particles.push_back(e);
}

void UpdateExplosion()
{
    for(auto e = particles.begin(); e != particles.end();) 
	{
        if(e->alpha <= 5) 
		{
            e = particles.erase(e);
            continue;
        }
        e->alpha -= 5;
        ++e;
    }
}

void NewAlien()
{
    if (game.alien_timer > 0)
        game.alien_timer--;
    else
    {
        ALIEN a;
        a.pos = Vec2(rand() %160, rand() %120);
        a.ani = rand() %4;
        p.pos.x > a.pos.x? a.sprite = 0: a.sprite = 1;
        a.alpha = 0;
        alien.push_back(a);
        game.alien--;
        game.alien_timer = 8;
    }
}

void UpdateAlien()
{
    for(auto a = alien.begin(); a != alien.end();) 
	{
        if (a->alpha == 255)
        {
            if(a->pos.x < p.pos.x + 3 && a->pos.x + 3 > p.pos.x && a->pos.y < p.pos.y + 8 && a->pos.y + 6 > p.pos.y)
            {
                a = alien.erase(a);
                continue;
            }

            float dx = p.pos.x - a->pos.x;
            float dy = p.pos.y - a->pos.y;
            float xy = sqrt((dx * dx) + (dy * dy)) * 5;
            a->vel = (p.pos - a->pos) / xy;
            a->vel.x > 0? a->sprite = 0: a->sprite = 1;
            a->pos += a->vel;
        }
        else
        {
            a->alpha += 3;
        }
        ++a;
    }
}

void UpdateAni(Timer &t)
{
    if (p.pos != p.pos0)
    {
        p.pos0 = p.pos;
        p.ani++;
        if (p.ani > 3)
            p.ani = 0;
    }
    else
    {
        p.ani = 0;
    }
    
    for(auto &a : alien)
	{
        if (a.alpha == 255)
        {
            a.ani++;
            if (a.ani > 3)
                a.ani = 0;
        }
    }
}

void UpdateShot()
{
    for(auto s = shot.begin(); s != shot.end();) 
	{
        if(s->pos.x < 0 || s->pos.x > 159 || s->pos.y < 0 || s->pos.y>119) 
		{
            s = shot.erase(s);
            continue;
        }
        bool hit = false;
        for(auto a = alien.begin(); a != alien.end();) 
	    {
            if(s->pos.x < a->pos.x + 5 && s->pos.x > a->pos.x && s->pos.y < a->pos.y + 6 && s->pos.y + 1 > a->pos.y)
            {
                NewExplosion(a->pos, a->sprite);
                a = alien.erase(a);
                s = shot.erase(s);
                hit = true;
                break;
            }
            ++a;
        }
        if (hit)
            continue;
        s->pos += s->vel;
        ++s;
    }
}

void UpdateControl()
{
    p.pos += Vec2(joystick.x, joystick.y);
    
    if (buttons & Button::DPAD_UP)
        p.pos.y--;
    else if (buttons & Button::DPAD_DOWN)
        p.pos.y++;
    if (buttons & Button::DPAD_LEFT)
        p.pos.x--;
    else if (buttons & Button::DPAD_RIGHT)
        p.pos.x++;

    if (p.pos.x < p.pos0.x)
        p.sprite = 8;
    else if (p.pos.x > p.pos0.x)
        p.sprite = 4;
    else
        p.sprite = 0;

    if (p.pos.x < -1)
        p.pos.x = -1;
    else if (p.pos.x > 155)
        p.pos.x = 155;
    if (p.pos.y < 0)
        p.pos.y = 0;
    else if (p.pos.y > 112)
        p.pos.y = 112;

    if (p.shot == 0)
    {
        short dx = 0;
        short dy = 0;

        if (buttons & Button::Y)
            dx = -2;
        else if (buttons & Button::A)
            dx = 2;
        if (buttons & Button::X)
            dy = -2;
        else if (buttons & Button::B)
            dy = 2;

        if (dx!=0 || dy!=0)// new shot
        {
            p.shot = 10;
            SHOT s;
            s.pos = p.pos + Vec2(2,4);
            s.vel = Vec2(dx,dy);
            shot.push_back(s);
        }
    }
    else
    {
        p.shot--;
    }
}

// init()
void init() 
{
    set_screen_mode(ScreenMode::lores);

    screen.sprites = Surface::load(sprites);

    ani_timer.init(UpdateAni, 100, -1);
    ani_timer.start();

    p.pos = Vec2(79,59);
}

// render()
void render(uint32_t time) 
{
    screen.clear();
    screen.mask = nullptr;

    //explosion
    for(auto &e : particles)
    {
        screen.alpha = e.alpha;
        screen.sprite(24 + e.sprite, Point(e.pos.x, e.pos.y));
    }

    for(auto &a : alien)
    {
        screen.alpha = a.alpha;
        screen.sprite(16 + (a.sprite * 4) + a.ani, Point(a.pos.x, a.pos.y));
    }

    screen.alpha = 255;
    screen.pen = Pen(255, 255, 255);

    for(auto &s : shot)
    {
        screen.pixel(Point(s.pos.x, s.pos.y));
    }

    screen.sprite(p.sprite + p.ani, Point(p.pos.x, p.pos.y));

//    if (game.state == 0)
//    std::string score_txt ("000000");
//    std::string score (std::to_string(game.score));
//    score_txt.erase(0, score.size());
//    screen.text(score_txt + score, font, Point(80, 1), true, TextAlign::top_center);        
//    screen.text(std::to_string(game.bonus), font, Point(159, 1), true, TextAlign::top_right);        

    screen.pen = Pen(0, 0, 0);
}

// update()
void update(uint32_t time) 
{
    if (game.state == 0) // game
    {
        UpdateControl();
        UpdateShot();
        UpdateAlien();
        UpdateExplosion();
        if (game.alien > 0)
            NewAlien();
        else if (alien.size() == 0)
        {
            game.alien = 10 + game.level;
            game.level++;
        }   
    }
}
