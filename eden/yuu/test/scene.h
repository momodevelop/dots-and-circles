#ifndef __TEST_SCENES_H__
#define __TEST_SCENES_H__


namespace scenes
{
	class IScene {
	public:
		virtual void init() = 0;
		virtual void update() = 0;
		virtual void cleanup() = 0;
		virtual bool done() = 0;
	};


}


#endif