#include "UI.h"

/* 
 * ---------------
 * | UIComponent |
 * ---------------
 */

// -- Public --

VkExtent2D UIComponent::screenextent = {0, 0};
UIPipelineInfo UIComponent::defaultgraphicspipeline = {};
UIImageInfo UIComponent::notex = {};
VkDescriptorSet UIComponent::defaultds = VK_NULL_HANDLE;
dfType UIComponent::defaultDrawFunc = nullptr;

void swap(UIComponent& c1, UIComponent& c2) {
	std::swap(c1.pcdata, c2.pcdata);
	std::swap(c1.graphicspipeline, c2.graphicspipeline);
	std::swap(c1.drawFunc, c2.drawFunc);
	std::swap(c1.onHover, c2.onHover);
	std::swap(c1.onHoverBegin, c2.onHoverBegin);
	std::swap(c1.onHoverEnd, c2.onHoverEnd);
	std::swap(c1.onClick, c2.onClick);
	std::swap(c1.onClickBegin, c2.onClickBegin);
	std::swap(c1.onClickEnd, c2.onClickEnd);
	std::swap(c1.ds, c2.ds);
	std::swap(c1.events, c2.events);
}

UIComponent& UIComponent::operator=(UIComponent rhs) {
	swap(*this, rhs);
	return *this;
}

void UIComponent::draw(const VkCommandBuffer& cb) const {
	if (display & UI_DISPLAY_FLAG_SHOW) {
		drawFunc(this, cb);
		for (const UIComponent* const c : getChildren()) {
			c->draw(cb);
		}
	}
}

void UIComponent::listenMousePos(UICoord mousepos, void* data) {
	if (!(display & UI_DISPLAY_FLAG_SHOW)) return;
	if (mousepos.x > this->getPos().x
		&& mousepos.y > this->getPos().y
		&& mousepos.x < this->getPos().x + this->getExt().x
		&& mousepos.y < this->getPos().y + this->getExt().y
		) {
		onHover(this, nullptr);
		if (!(events & UI_EVENT_FLAG_HOVER)) {
			onHoverBegin(this, nullptr);
			events |= UI_EVENT_FLAG_HOVER;
		}
		for (UIComponent* c : _getChildren()) c->listenMousePos(mousepos, data);
	} else if (events & UI_EVENT_FLAG_HOVER) {
		onHoverEnd(this, nullptr);
		events &= ~UI_EVENT_FLAG_HOVER;
		for (UIComponent* c : _getChildren()) c->listenMousePos(mousepos, data);
	} 
	if (display & UI_DISPLAY_FLAG_OVERFLOWING_CHILDREN) {
		for (UIComponent* c : _getChildren()) c->listenMousePos(mousepos, data);
	}
}

void UIComponent::listenMouseClick(bool click, void* data) {
	if (!(display & UI_DISPLAY_FLAG_SHOW)) return;
	if ((events & UI_EVENT_FLAG_HOVER) && click) {
		onClick(this, nullptr);
		if (!(events & UI_EVENT_FLAG_CLICK)) {
			onClickBegin(this, nullptr);
			events |= UI_EVENT_FLAG_CLICK;
		}
		for (UIComponent* c : _getChildren()) c->listenMouseClick(click, data);
	} else if (events & UI_EVENT_FLAG_CLICK) {
		onClickEnd(this, nullptr);
		events &= ~UI_EVENT_FLAG_CLICK;
		for (UIComponent* c : _getChildren()) c->listenMouseClick(click, data);
	} 
	if (display & UI_DISPLAY_FLAG_OVERFLOWING_CHILDREN) {
		for (UIComponent* c : _getChildren()) c->listenMouseClick(click, data);
	}
}

void UIComponent::setPos(UICoord p) {
	UICoord diff = p - pcdata.position;
	pcdata.position = p;
	for (UIComponent* c : _getChildren()) c->setPos(c->getPos() + diff);
}

void UIComponent::setGraphicsPipeline(const UIPipelineInfo& p) {
	graphicspipeline = p;
	for (UIComponent* c : _getChildren()) c->setGraphicsPipeline(p);
}

void UIComponent::show() {
	// should technically re-listen for mousepos & click
	setDisplayFlag(UI_DISPLAY_FLAG_SHOW);
}

void UIComponent::hide() {
	unsetDisplayFlag(UI_DISPLAY_FLAG_SHOW);
	if (events & UI_EVENT_FLAG_HOVER) {
		events &= ~UI_EVENT_FLAG_HOVER;
		onHoverEnd(this, nullptr);
	}
	if (events & UI_EVENT_FLAG_CLICK) {
		events &= ~UI_EVENT_FLAG_CLICK;
		onClickEnd(this, nullptr);
	}
}

// -- Protected --

UIComponent::UIComponent(UIComponent&& rhs) noexcept :
		pcdata(rhs.pcdata),
		graphicspipeline(defaultgraphicspipeline),
		drawFunc(rhs.drawFunc),
		onHover(rhs.onHover),
		onHoverBegin(rhs.onHoverBegin),
		onHoverEnd(rhs.onHoverEnd),
		onClick(rhs.onClick),
		onClickBegin(rhs.onClickBegin),
		onClickEnd(rhs.onClickEnd),
		ds(rhs.ds),
		events(rhs.events),
		display(rhs.display) {
	// TODO: figure out if this body is neccesary
	// TODO: figure out if list init should use std::move
	rhs.pcdata = (UIPushConstantData){};
	rhs.drawFunc = nullptr;
	rhs.onHover = nullptr;
	rhs.onHoverBegin = nullptr;
	rhs.onHoverEnd = nullptr;
	rhs.onClick = nullptr;
	rhs.onClickBegin = nullptr;
	rhs.onClickEnd = nullptr;
	rhs.ds = VK_NULL_HANDLE;
	rhs.events = UI_EVENT_FLAG_NONE;
	rhs.display = UI_DISPLAY_FLAG_SHOW;
}

// -- Private --

cfType UIComponent::defaultOnHover = [] (UIComponent* self, void* d) {};
cfType UIComponent::defaultOnHoverBegin = [] (UIComponent* self, void* d) {
	self->pcdata.bgcolor = UI_DEFAULT_HOVER_BG_COLOR;
};
cfType UIComponent::defaultOnHoverEnd = [] (UIComponent* self, void* d) {
	self->pcdata.bgcolor = UI_DEFAULT_BG_COLOR;
};
cfType UIComponent::defaultOnClick = [] (UIComponent* self, void* d) {};
cfType UIComponent::defaultOnClickBegin = [] (UIComponent* self, void* d) {
	self->pcdata.bgcolor = UI_DEFAULT_CLICK_BG_COLOR;
};
cfType UIComponent::defaultOnClickEnd = [] (UIComponent* self, void* d) {
	self->pcdata.bgcolor = UI_DEFAULT_BG_COLOR;
};

/*
 * ---------------
 * | UIContainer |
 * ---------------
 */

// -- Public --

UIContainer::~UIContainer() {
	for (UIComponent* c : children) delete c;
}

std::vector<const UIComponent*> UIContainer::getChildren() const {
	std::vector<const UIComponent*> result = {};
	for (size_t i = 0; i < children.size(); i++) result.push_back(children[i]);
	return result;
}

/*
void UIContainer::addChild(const UIComponent& c) {
	children.push_back(new UIComponent);
	*children.back() = c;
}
*/

// -- Private --

std::vector<UIComponent*> UIContainer::_getChildren() {
	return children;
}

/*
 * -----------
 * | UIImage |
 * -----------
 */

tfType UIImage::texLoadFunc = nullptr; 
tdfType UIImage::texDestroyFunc = nullptr;
std::map<VkImage, uint8_t> UIImage::imgusers = {};

// -- Public --

UIImage::UIImage() {
#ifdef VERBOSE_IMAGE_OBJECTS
	std::cout << "UIImage()" << std::endl;
#endif
}

UIImage::UIImage(const UIImage& rhs) :
		tex(rhs.tex),
		UIComponent(rhs) {
	imgusers[tex.image]++;
#ifdef VERBOSE_IMAGE_OBJECTS
	std::cout << "UIImage(const UIImage&)\n";
	std::cout << (int)imgusers[tex.image] << " users of " << tex.image << std::endl;
#endif
}

UIImage::UIImage(UIImage&& rhs) noexcept :
	tex(std::move(rhs.tex)),
	UIComponent(rhs) {
#ifdef VERBOSE_IMAGE_OBJECTS
	std::cout << "UIImage(UIImage&&)\n";
	std::cout << (int)imgusers[tex.image] << " users of " << tex.image << std::endl;
#endif
}

UIImage::UIImage(UICoord p) : UIComponent(p, UICoord{0, 0}) {
	UIImage();
#ifdef VERBOSE_IMAGE_OBJECTS
	std::cout << "UIImage(UICoord)\n";
	std::cout << (int)imgusers[tex.image] << " users of " << tex.image << std::endl;
#endif
}

UIImage::~UIImage() {
#ifdef VERBOSE_IMAGE_OBJECTS
	std::cout << "~UIImage()\n";
	std::cout << (int)imgusers[tex.image] << " users of " << tex.image << std::endl;
#endif
	if (tex.image != VK_NULL_HANDLE) {
		imgusers[tex.image]--;
		if (imgusers[tex.image] == 0) texDestroyFunc(this);
	}
}

void swap(UIImage& t1, UIImage& t2) {
	swap(static_cast<UIComponent&>(t1), static_cast<UIComponent&>(t2));
	std::swap(t1.tex, t2.tex);
}

UIImage& UIImage::operator=(UIImage rhs) {
	swap(*this, rhs);
#ifdef VERBOSE_IMAGE_OBJECTS
	std::cout << "Image& = Image\n";
	std::cout << (int)imgusers[tex.image] << " users of " << tex.image << std::endl;
#endif
	return *this;
}

void UIImage::setTex(const UIImageInfo& i) {
	if (tex.image != i.image) {
		if (tex.image != VK_NULL_HANDLE) imgusers[tex.image]--;
		if (i.image != VK_NULL_HANDLE) imgusers[i.image]++;
	}
	tex = i;
}

// -- Private --

/* 
 * ----------
 * | UIText |
 * ----------
 */

FT_Library UIText::ft = nullptr;
FT_Face UIText::typeface = nullptr;

// -- Public --

UIText::UIText() : text(L""), UIImage() {
	if (!ft) {
		ft = FT_Library();
		FT_Init_FreeType(&ft);
	}
	if (!typeface) {
		FT_New_Face(ft, UI_DEFAULT_SANS_FILEPATH, UI_DEFAULT_SANS_IDX, &typeface); 
	}
	pcdata.blend = true;
}

UIText::UIText(std::wstring t) : UIText() {
	text = t;
	genTex();
}

UIText::UIText(std::wstring t, UICoord p) : UIText() {
	setPos(p);
	text = t;
	genTex();
}

void swap(UIText& t1, UIText& t2) {
	swap(static_cast<UIImage&>(t1), static_cast<UIImage&>(t2));
	std::swap(t1.text, t2.text);
}

UIText& UIText::operator=(UIText rhs) {
	swap(*this, rhs);
	return *this;
}

void UIText::setDS(VkDescriptorSet d) {
	ds = d;
	genTex();
}

void UIText::setText(std::wstring t) {
	text = t;
	genTex();
}

// -- Private --

void UIText::genTex() {
	const uint32_t pixelscale = 64;
	// FT_Set_Char_Size(typeface, 0, 50 * pixelscale, 0, 0);
	FT_Set_Pixel_Sizes(typeface, 0, 32);
	uint32_t maxlinelength = 0, linelengthcounter = 0, numlines = 1;
	for (char c : text) {
		if (c == '\n') {
			if (linelengthcounter > maxlinelength) maxlinelength = linelengthcounter;
			linelengthcounter = 0;
			numlines++;
			continue;
		}
		FT_Load_Char(typeface, c, FT_LOAD_DEFAULT);
		linelengthcounter += typeface->glyph->metrics.horiAdvance;
	}
	if (linelengthcounter > maxlinelength) maxlinelength = linelengthcounter;

	FT_Size_Metrics m = typeface->size->metrics;
	const uint32_t hres = maxlinelength / pixelscale, vres = numlines * m.height / pixelscale;
	// TODO: switch all hres, vres to this extent
	UIImageInfo temp = getTex();
	temp.extent = {hres, vres};
	setTex(temp);
	unorm* texturedata = (unorm*)malloc(hres * vres * sizeof(float));
	memset(&texturedata[0], 0.0f, hres * vres * sizeof(float));
	// TODO: should this be int or float?
	UICoord penposition(0, vres - m.ascender / pixelscale);

	FT_Glyph_Metrics gm;
	for (char c : text) {
		if (c == '\n') {
			penposition.y -= m.height / pixelscale;
			penposition.x = 0;
			continue;
		}
		FT_Load_Char(typeface, c, FT_LOAD_RENDER);
		if (typeface->glyph) FT_Render_Glyph(typeface->glyph, FT_RENDER_MODE_NORMAL);
		gm = typeface->glyph->metrics;
		penposition += UICoord(gm.horiBearingX, gm.horiBearingY) / pixelscale;
		unsigned char* bitmapbuffer = typeface->glyph->bitmap.buffer;
		// TODO: turn into UICoord
		uint32_t xtex = 0, ytex = 0;
		for (uint32_t y = 0; y < typeface->glyph->bitmap.rows; y++) {
			for (uint32_t x = 0; x < typeface->glyph->bitmap.width; x++) {
				xtex = (uint32_t)penposition.x + x;
				ytex = (uint32_t)penposition.y - y;
				texturedata[ytex * hres + xtex] = *bitmapbuffer++;
			}
		}
		penposition += UICoord(gm.horiAdvance - gm.horiBearingX, -gm.horiBearingY) / pixelscale;
	}

	pcdata.extent = UICoord(hres, vres);

	// TODO: allow for regeneration of (static size) texture
	texLoadFunc(this, texturedata);
	free(texturedata);
}

/* 
 * --------------
 * | UIDropdown |
 * --------------
 */

// -- Public --

UIDropdown::UIDropdown(UIDropdown&& rhs) noexcept :
	options(rhs.options),
	unfolded(std::move(rhs.unfolded)),
	otherpos(std::move(rhs.otherpos)),
	otherext(std::move(rhs.otherext)),
	UIComponent(rhs) {
	rhs.options = {};
}

UIDropdown::UIDropdown(std::vector<std::wstring> o) : UIComponent() {
	display |= UI_DISPLAY_FLAG_OVERFLOWING_CHILDREN;
	setOptions(o);
}

UIDropdown::UIDropdown(std::vector<std::wstring> o, UICoord p, UICoord e) : UIComponent(p, e) {
	display |= UI_DISPLAY_FLAG_OVERFLOWING_CHILDREN;
	setOptions(o);
}

std::vector<const UIComponent*> UIDropdown::getChildren() const {
	std::vector<const UIComponent*> result = {};
	for (size_t i = 0; i < options.size(); i++) result.push_back(&options[i]);
	return result;
}

void UIDropdown::setPos(UICoord p) {
	UICoord diff = p - pcdata.position;
	static_cast<UIComponent*>(this)->setPos(p);
	otherpos += diff;
}

void UIDropdown::setExt(UICoord e) {
	otherext.y = otherext.y - getExt().y + e.y;
	static_cast<UIComponent*>(this)->setExt(e);
	if (getExt().x > otherext.x) otherext.x = getExt().x;
}

void UIDropdown::fold() {
	if (unfolded) {
		for (UIText& o : options) o.hide();
		std::swap(pcdata.position, otherpos);
		std::swap(pcdata.extent, otherext);
	}
	unfolded = false;
}

void UIDropdown::unfold() {
	if (!unfolded) {
		for (UIText& o : options) o.show();
		std::swap(pcdata.position, otherpos);
		std::swap(pcdata.extent, otherext);
	}
	unfolded = true;
}

// -- Protected --

std::vector<UIComponent*> UIDropdown::_getChildren() {
	std::vector<UIComponent*> result = {};
	for (size_t i = 0; i < options.size(); i++) result.push_back(&options[i]);
	return result;
}

// -- Private --

void UIDropdown::setOptions(std::vector<std::wstring>& o) {
	fold();
	otherext = {0, 0};
	options = std::vector<UIText>();
	float height = getPos().y;
	for (std::wstring& opt : o) {
		options.emplace_back(opt);
		options.back().setPos(this->getPos() + UICoord(0, height - options.back().getExt().y));
		height = options.back().getPos().y;
		if (options.back().getExt().x > otherext.x) otherext.x = options.back().getExt().x;
		options.back().hide();
		options.back().setGraphicsPipeline(graphicspipeline);
	}
	otherext.y = getPos().y + getExt().y - options.back().getPos().y;
	otherpos = options.back().getPos();
}

/* 
 * ---------------------
 * | UIDropdownButtons |
 * ---------------------
 */

// -- Public --

UIDropdownButtons::UIDropdownButtons(UIDropdownButtons&& rhs) noexcept :
	UIDropdown(rhs) {
	title = std::move(rhs.title);
}

UIDropdownButtons::UIDropdownButtons(std::wstring t) : title(t), UIDropdown() {
	// TODO: consolidate code in below two constructors
	this->setExt(title.getExt());
	/*
	otherext.y += getExt().y;
	if (getExt().x > otherext.x) otherext.x = getExt().x;
	*/
	/*
	setOnClickBegin([this] (UIComponent* self, void* d) {
		for (UIText& o : options) {
			o.setDisplayFlag(UI_DISPLAY_FLAG_SHOW);
		}
		std::swap(pcdata.position, otherpos);
		std::swap(pcdata.extent, otherext);
	});
	*/
}

UIDropdownButtons::UIDropdownButtons(std::wstring t, std::vector<std::wstring> o) : title(t), UIDropdown(o) {
	this->setExt(title.getExt());
	/*
	otherext.y += getExt().y;
	if (getExt().x > otherext.x) otherext.x = getExt().x;
	*/
	setOnClickBegin([] (UIComponent* self, void* d) {
		UIDropdownButtons* ddbself = static_cast<UIDropdownButtons*>(self);
		ddbself->unfold();
	});
	setOnHoverEnd([] (UIComponent* self, void* d) {
		UIDropdownButtons* ddbself = static_cast<UIDropdownButtons*>(self);
		ddbself->fold();
	});
}

std::vector<const UIComponent*> UIDropdownButtons::getChildren() const {
	std::vector<const UIComponent*> result = UIDropdown::getChildren();
	result.insert(result.begin(), &title);
	return result;
}

// -- Private --

std::vector<UIComponent*> UIDropdownButtons::_getChildren() {
	std::vector<UIComponent*> result = UIDropdown::_getChildren();
	result.insert(result.begin(), &title);
	return result;
}

/* 
 * ----------------------
 * | UIDropdownSelector |
 * ----------------------
 */

// -- Public --

// -- Private --


/* 
 * ------------
 * | UIRibbon |
 * ------------
 */

// -- Public --

UIRibbon::UIRibbon() : UIComponent(), options({}) {
	setPos(UICoord(0, screenextent.height - 50));
	setExt(UICoord(screenextent.width, 50));
	display |= UI_DISPLAY_FLAG_OVERFLOWING_CHILDREN;
}

std::vector<const UIComponent*> UIRibbon::getChildren() const {
	std::vector<const UIComponent*> result;
	for (size_t i = 0; i < options.size(); i++) result.push_back(&options[i]);
	return result;
}

void UIRibbon::addOption(std::wstring name) {
	float xlen = options.size() ? options.back().getPos().x + options.back().getExt().x : 0;
	options.emplace_back(name);
	options.back().setPos(UICoord(50 + xlen, this->getPos().y));
	options.back().setExt(options.back().getExt() + UICoord(50, 0));
	options.back().setGraphicsPipeline(graphicspipeline);
}

void UIRibbon::addOption(UIDropdownButtons&& o) {
	float xlen = options.size() ? options.back().getPos().x + options.back().getExt().x : 0;
	options.emplace_back(o);
	options.back().setPos(UICoord(50 + xlen, this->getPos().y));
	options.back().setExt(options.back().getExt() + UICoord(50, 0));
	options.back().setGraphicsPipeline(graphicspipeline);
}

void UIRibbon::addOption(std::wstring t, std::vector<std::wstring> o) {
	// TODO: consolidate in addOptionInternal
	float xlen = options.size() ? options.back().getPos().x + options.back().getExt().x : 0;
	options.emplace_back(t, o);
	options.back().setPos(UICoord(50 + xlen, this->getPos().y));
	options.back().setExt(options.back().getExt() + UICoord(50, 0));
	options.back().setGraphicsPipeline(graphicspipeline);
}

// -- Private --

std::vector<UIComponent*> UIRibbon::_getChildren() {
	std::vector<UIComponent*> result;
	for (size_t i = 0; i < options.size(); i++) result.push_back(&options[i]);
	return result;
}
