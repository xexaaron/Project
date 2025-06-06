#include "Widget/WidgetSwitcher.h"

namespace aby::ui {

	WidgetSwitcher::WidgetSwitcher(App* app, const Transform& transform, const ImageStyle& tab_style) :
		Widget(transform, "WidgetSwitcher"),
		m_ActiveWidget(1),
		m_TabSize(30, 20),
		m_TabStyle(tab_style),
		m_App(app),
		bCreatedTabs(false)
	{
		bScalesWithWindow = true;
	}
	
	Ref<WidgetSwitcher> WidgetSwitcher::create(App* app, const Transform& transform, const ImageStyle& tab_style) {
		return create_ref<WidgetSwitcher>(app, transform, tab_style);
	}

	void WidgetSwitcher::on_create(App* app, bool) {
		ensure_tabs();
		Super::on_create(app, false);
	}

	std::size_t WidgetSwitcher::add_child(Ref<Widget> child) {
		ensure_tabs();
		std::size_t idx = Super::add_child(child);
		auto tabs = get_tabs();
		Transform tab_tf;
		tab_tf.size = m_TabSize;
		TextInfo text_info;
		text_info.alignment = ETextAlignment::CENTER;
		text_info.color     = { 1.f, 1.f, 1.f, 1.f };
		text_info.scale		= 1.f;
		text_info.text		= child->name();
		Ref<Button> tab = Button::create(tab_tf, ButtonStyle::dark_mode(), text_info, false);
		tabs->add_child(tab);
		tab->on_create(m_App, false);
		auto child_pos = m_Transform.position;
		child_pos.y -= m_TabSize.y;
		child->set_position(child_pos);
		return idx;
	}

	void WidgetSwitcher::on_tick(App* app, Time deltatime) {
		auto child_pos = m_Transform.position;
		child_pos.y   -= m_TabSize.y;
		active_tab()->set_position(child_pos);
		Super::on_tick(app, deltatime);
	}

	void WidgetSwitcher::for_each(std::function<void(Ref<Widget>)> fn) {
		ABY_ASSERT(m_ActiveWidget < m_Children.size(), "Out of bounds");
		fn(m_Children[0]);
		fn(m_Children[m_ActiveWidget]);
	}

	Ref<Widget> WidgetSwitcher::active_tab() {
		ABY_ASSERT(m_ActiveWidget < m_Children.size(), "Out of bounds");
		return m_Children[m_ActiveWidget];
	}

	Ref<LayoutContainer> WidgetSwitcher::get_tabs() {
		ABY_ASSERT(!m_Children.empty(), "Out of bounds");
		return std::static_pointer_cast<LayoutContainer>(m_Children[0]);
	}

	void WidgetSwitcher::set_active(std::size_t index) {
		m_ActiveWidget = std::clamp(index, std::size_t(1), m_Children.size());
	}

	const glm::vec2& WidgetSwitcher::tab_size() const {
		return m_TabSize;
	}

	void WidgetSwitcher::ensure_tabs() {
		if (bCreatedTabs) return;

		Transform tab_transform = m_Transform;
		tab_transform.size.y = m_TabSize.y;
		tab_transform.anchor.position = EAnchor::TOP_LEFT;
		tab_transform.anchor.offset = { 0, 0 };

		auto container = LayoutContainer::create(tab_transform, m_TabStyle, EDirection::HORIZONTAL);
		container->set_parent(shared_from_this());
		m_Children.insert(m_Children.begin(), container);
		bCreatedTabs = true;
	}

}