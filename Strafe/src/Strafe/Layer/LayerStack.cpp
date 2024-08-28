

#include "strafepch.h"

#include "LayerStack.h"

#include "Layer.h"

namespace strafe
{
	void LayerStack::Init()
	{
		for(auto& it : m_Layers)
		{
			it->Init();
		}
	}

	void LayerStack::Shutdown()
	{
		for(auto& it : m_Layers)
		{
			it->Shutdown();
		}
	}

	void LayerStack::PushLayer(std::shared_ptr<Layer> layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(std::shared_ptr<Layer> overlay)
	{
		m_Layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(std::shared_ptr<Layer> layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
		if (it != m_Layers.begin() + m_LayerInsertIndex)
		{
			layer->Shutdown();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}

	void LayerStack::PopOverlay(std::shared_ptr<Layer> overlay)
	{
		auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);
		if (it != m_Layers.end())
		{
			overlay->Shutdown();
			m_Layers.erase(it);
		}
	}
}
