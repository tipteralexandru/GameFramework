//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//
// Desc: This file stores the player object class. This class performs tasks
//       such as player movement, some minor physics as well as rendering.
//
// Original design by Adam Hoult & Gary Simmons. Modified by Mihai Popescu.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// CPlayer Specific Includes
//-----------------------------------------------------------------------------
#include "CPlayer.h"

//-----------------------------------------------------------------------------
// Name : CPlayer () (Constructor)
// Desc : CPlayer Class Constructor
//-----------------------------------------------------------------------------
CPlayer::CPlayer()
{
	//m_pSprite = new Sprite("data/planeimg.bmp", "data/planemask.bmp");
	m_pSprite = new Sprite("data/planeimgandmask.bmp", RGB(0xff,0x00, 0xff));
	m_eSpeedState = SPEED_STOP;
	m_fTimer = 0;

	// Animation frame crop rectangle
	RECT r;
	r.left = 0;
	r.top = 0;
	r.right = 128;
	r.bottom = 128;

	m_pExplosionSprite	= new AnimatedSprite("data/explosion.bmp", "data/explosionmask.bmp");
	m_pExplosionSprite->Initialize(r, 16, 1 / 16.f);
	m_bExplosion		= false;
}

//-----------------------------------------------------------------------------
// Name : ~CPlayer () (Destructor)
// Desc : CPlayer Class Destructor
//-----------------------------------------------------------------------------
CPlayer::~CPlayer()
{
	delete m_pSprite;
	delete m_pExplosionSprite;
}

void CPlayer::Init(HDC hdc, const Vec2& position)
{
	m_pSprite->Setup(hdc);
	m_pExplosionSprite->Setup(hdc);
	myPosition = position;
}

void CPlayer::Update(float dt)
{
	CGameObject::Update(dt);

	// Update sprites
	m_pSprite->myPosition = myPosition;
	m_pExplosionSprite->Update(dt);

	// Get velocity
	double v = myVelocity.Magnitude();

	// NOTE: for each async sound played Windows creates a thread for you
	// but only one, so you cannot play multiple sounds at once.
	// This creation/destruction of threads also leads to bad performance
	// so this method is not recommended to be used in complex projects.

	// update internal time counter used in sound handling (not to overlap sounds)
	m_fTimer += dt;

	// handle plane explosion
	if(m_bExplosion && m_pExplosionSprite->IsFinished())
	{
		m_bExplosion = false;
		myVelocity = Vec2(0,0);
		m_eSpeedState = SPEED_STOP;
	}

	// A FSM is used for sound manager 
	switch(m_eSpeedState)
	{
	case SPEED_STOP:
		if(v > 35.0f)
		{
			m_eSpeedState = SPEED_START;
			PlaySound("data/jet-start.wav", NULL, SND_FILENAME | SND_ASYNC);
			m_fTimer = 0;
		}
		break;
	case SPEED_START:
		if(v < 25.0f)
		{
			m_eSpeedState = SPEED_STOP;
			PlaySound("data/jet-stop.wav", NULL, SND_FILENAME | SND_ASYNC);
			m_fTimer = 0;
		}
		else
			if(m_fTimer > 1.f)
			{
				PlaySound("data/jet-cabin.wav", NULL, SND_FILENAME | SND_ASYNC);
				m_fTimer = 0;
			}
		break;
	}

	// NOTE: For sound you also can use MIDI but it's Win32 API it is a bit hard
	// see MSDN reference: http://msdn.microsoft.com/en-us/library/ms711640.aspx
	// In this case you can use a C++ wrapper for it. See the following article:
	// http://www.codeproject.com/KB/audio-video/midiwrapper.aspx (with code also)
}

void CPlayer::Draw(HDC hdc) const
{
	if(!m_bExplosion)
		m_pSprite->Draw(hdc);
	else
		m_pExplosionSprite->Draw(hdc);
}

void CPlayer::Move(ULONG ulDirection)
{
	myAcceleration = Vec2();

	if( ulDirection & CPlayer::DIR_LEFT )
		myAcceleration.x = -2 * GCONST;

	if( ulDirection & CPlayer::DIR_RIGHT )
		myAcceleration.x = 2 * GCONST;

	if( ulDirection & CPlayer::DIR_FORWARD )
		myAcceleration.y = -2 * GCONST;

	if( ulDirection & CPlayer::DIR_BACKWARD )
		myAcceleration.y = 2 * GCONST;
}

void CPlayer::Explode()
{
	m_pExplosionSprite->myPosition = m_pSprite->myPosition;
	m_pExplosionSprite->Play();
	PlaySound("data/explosion.wav", NULL, SND_FILENAME | SND_ASYNC);
	m_bExplosion = true;
}