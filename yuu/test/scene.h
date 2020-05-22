#ifndef __TEST_SCENES_H__
#define __TEST_SCENES_H__


#include <unordered_map>
#include "ryoji/allocators/linear-allocator.h"
namespace scenes
{
	class IScene {
	public:
		virtual void init() = 0;
		virtual IScene* update() = 0;
		virtual void uninit() = 0;
		virtual ~IScene() = default;
	};


	class SceneManager {
		std::unordered_map<std::string, IScene*> database;
		IScene* currentScene;
		IScene* nextScene;
	public:
		template<typename SceneType>
		void add(const std::string& key) {
			database.emplace(key, std::make_unique<SceneType>());
		}

		void run() {
			while (currentScene || nextScene ) {
				if (nextScene != currentScene) {
					if (currentScene) {
						currentScene->uninit();
					}
					currentScene = nextScene;
					if (currentScene)
						currentScene->init();
				}

				if (currentScene) {
					auto* scene = currentScene->update();
					if (scene != currentScene) {
						nextScene = scene;
					}
				}
			}
		}

		void setNextScene(IScene* scene) {
			nextScene = scene;
		}



	};



}


#endif