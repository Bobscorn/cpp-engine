#pragma once

namespace Drawing
{
	class DrawCallRenderer;
	struct DrawCallv2;

	class DrawCallReference
	{
	protected:
		size_t _key = 0ull;
		DrawCallRenderer* _ren = nullptr;

	public:
		DrawCallReference() = default;
		DrawCallReference(size_t key, DrawCallRenderer* ren) : _key(key), _ren(ren) {}
		DrawCallReference(DrawCallReference&& other);
		DrawCallReference(const DrawCallReference& other) = delete;
		~DrawCallReference();

		DrawCallReference& operator=(DrawCallReference&& other);
		DrawCallReference& operator=(const DrawCallReference& other) = delete;

		void Reset(size_t key = 0ull, DrawCallRenderer* ren = nullptr);

		inline size_t GetKey() const { return _key; }

		const DrawCallv2* GetDrawCall() const;
	};
}