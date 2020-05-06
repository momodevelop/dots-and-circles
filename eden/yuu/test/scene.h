#ifndef __TEST_SCENES_H__
#define __TEST_SCENES_H__


#include <unordered_map>
#include "ryoji/allocators/linear-allocator.h"
namespace scenes
{
	template<typename Key>
	class SceneManager {
		class IScene {
		public:
			virtual void init() = 0;
			virtual void update() = 0;
			virtual void uninit() = 0;
			virtual ~IScene() = default;
		};

		template<typename SceneImpl>
		class Model : public IScene {
			SceneImpl instance;
		public:
			template<typename ...Args>
			Model(Args&& ...args) : instance(std::forward<Args>(args)...) {}
			virtual void init() override { instance.init(); }
			virtual void update() override { instance.update(); }
			virtual void uninit() override { instance.uninit(); }
			virtual ~Model() = default;
		};

		std::unordered_map<Key, IScene*> scenes{};
		IScene* currentScene{ nullptr };
		IScene* nextScene{ nullptr };

	public:
		template<typename SceneImpl, typename ... Args>
		void registerScene(Key key, Args&&... args) {
			auto model = new Model<SceneImpl>(std::forward(args)...);
			scenes.emplace(key, model);
		}

		void update() {
			if (currentScene)
				currentScene->update();

			if (nextScene) {
				if (currentScene)
					currentScene->uninit();
				currentScene = nextScene;
				currentScene->init();
				nextScene = nullptr;
			}
		}

		bool changeScene(Key key) {
			auto itr = scenes.find(key);
			if (itr != scenes.end()) {
				nextScene = itr->second;
				return true;
			}
			return false;
		}

		bool isDone() {
			return false;
		}
	};

}


#endif