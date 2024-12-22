#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <functional>
#include <vulkan/vulkan.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef __APPLE__
#define UI_DEFAULT_SANS_FILEPATH "/System/Library/fonts/Avenir Next.ttc"
#define UI_DEFAULT_SANS_IDX 2
#define UI_DEFAULT_SERIF_FILEPATH "/System/Library/fonts/supplemental/Times New Roman.ttf"
#define UI_DEFAULT_SERIF_IDX 0
#else
#define UI_DEFAULT_SANS_FILEPATH "/usr/share/fonts/truetype/noto/NotoSansDisplay-Regular.ttf"
#define UI_DEFAULT_SANS_IDX 0
#endif
#define UI_DEFAULT_BG_COLOR (UIColor){0.3, 0.3, 0.3, 1}
#define UI_DEFAULT_HOVER_BG_COLOR (UIColor){0.4, 0.4, 0.4, 1}
#define UI_DEFAULT_CLICK_BG_COLOR (UIColor){1, 0.4, 0.4, 1}

// #define VERBOSE_IMAGE_OBJECTS

class UIComponent;

class UIImage;

typedef unsigned char unorm;

typedef std::function<void (const UIComponent* const, const VkCommandBuffer&)> dfType;

typedef std::function<void (UIImage*, void*)> tfType;

typedef std::function<void (UIImage*)> tdfType;

typedef std::function<void (UIComponent*, void*)> cfType;

typedef struct UIPipelineInfo {
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkDescriptorSetLayout dsl = VK_NULL_HANDLE;
	VkDescriptorSetLayoutCreateInfo descsetlayoutci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
	};
	VkPushConstantRange pushconstantrange = {};
	VkSpecializationInfo specinfo = {};
} UIPipelineInfo;

typedef struct UIImageInfo {
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkExtent2D extent = {0, 0};
	VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkFormat format = VK_FORMAT_R8_UNORM;
} UIImageInfo;

// used as pixel coords starting bottom left (although sub-pixel values should still compute correctly)
typedef struct UICoord {
	float x, y;

	UICoord() {
		x = 0;
		y = 0;
	}
	UICoord(float x1, float y1) {
		x = x1;
		y = y1;
	}
	UICoord(float xy) {
		x = xy;
		y = xy;
	}

	UICoord& operator=(const UICoord& rhs) {
		x = rhs.x;
		y = rhs.y;
		return *this;
	}
	UICoord operator+(const UICoord& rhs) const {
		return {x + rhs.x, y + rhs.y};
	}
	UICoord& operator+=(const UICoord& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	UICoord operator-(const UICoord& rhs) const {
		return {x - rhs.x, y - rhs.y};
	}
	UICoord& operator-=(const UICoord& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	UICoord operator/(float rhs) const {
		return {x / rhs, y / rhs};
	}
} UICoord;

typedef struct UITexelCoord {
	uint32_t x, y;

	UITexelCoord& operator+=(const UITexelCoord& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	UITexelCoord operator-(const UITexelCoord& rhs) const {
		return {x - rhs.x, y - rhs.y};
	}
	UITexelCoord& operator-=(const UITexelCoord& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	UITexelCoord operator/(uint32_t rhs) const {
		return {x / rhs, y / rhs};
	}
} UITexelCoord;

// could be made into a unorm...
typedef struct UIColor {
	float r, g, b, a;
} UIColor;

typedef enum UIPushConstantFlagBits {
	UI_PC_FLAG_NONE =  0x00,
	UI_PC_FLAG_BLEND = 0x01,
	UI_PC_FLAG_TEX =   0x02
} UIPushConstantFlagBits;
typedef uint32_t UIPushConstantFlags;

typedef struct UIPushConstantData {
	UIColor bgcolor = UI_DEFAULT_BG_COLOR;
	UICoord position = {0, 0}, extent = {0, 0};
	UIPushConstantFlags flags = UI_PC_FLAG_NONE;
} UIPushConstantData;

typedef uint8_t UIEventFlags;

typedef enum UIEventFlagBits {
	UI_EVENT_FLAG_NONE =  0x00,
	UI_EVENT_FLAG_HOVER = 0x01,
	UI_EVENT_FLAG_CLICK = 0x02
} UIEventFlagBits;

typedef uint8_t UIDisplayFlags;

typedef enum UIDisplayFlagBits {
	UI_DISPLAY_FLAG_SHOW =                 0x01,
	UI_DISPLAY_FLAG_OVERFLOWING_CHILDREN = 0x02
} UIDisplayFlagBits;

class UIComponent {
public:
	UIComponent() : 
		pcdata({UI_DEFAULT_BG_COLOR, {0, 0}, {0, 0}, UI_PC_FLAG_NONE}),
		graphicspipeline(defaultgraphicspipeline),
		drawFunc(defaultDrawFunc), 
		onHover(defaultOnHover),
		onHoverBegin(defaultOnHoverBegin),
		onHoverEnd(defaultOnHoverEnd),
		onClick(defaultOnClick),
		onClickBegin(defaultOnClickBegin),
		onClickEnd(defaultOnClickEnd),
		ds(defaultds), 
		events(UI_EVENT_FLAG_NONE),
		display(UI_DISPLAY_FLAG_SHOW) {}
	UIComponent(UICoord p, UICoord e) : 
		pcdata({UI_DEFAULT_BG_COLOR, p, e, UI_PC_FLAG_NONE}), 
		graphicspipeline(defaultgraphicspipeline),
		drawFunc(defaultDrawFunc), 
		onHover(defaultOnHover),
		onHoverBegin(defaultOnHoverBegin),
		onHoverEnd(defaultOnHoverEnd),
		onClick(defaultOnClick),
		onClickBegin(defaultOnClickBegin),
		onClickEnd(defaultOnClickEnd),
		ds(defaultds),
		events(UI_EVENT_FLAG_NONE),
		display(UI_DISPLAY_FLAG_SHOW) {}
	UIComponent(const UIComponent& rhs) :
		pcdata(rhs.pcdata),
		graphicspipeline(rhs.graphicspipeline),
		drawFunc(rhs.drawFunc),
		onHover(rhs.onHover),
		onHoverBegin(rhs.onHoverBegin),
		onHoverEnd(rhs.onHoverEnd),
		onClick(rhs.onClick),
		onClickBegin(rhs.onClickBegin),
		onClickEnd(rhs.onClickEnd),
		ds(rhs.ds),
		events(rhs.events),
		display(rhs.display) {}
	UIComponent(UIComponent&& rhs) noexcept;
	virtual ~UIComponent() = default;

	// this should probably be protected no?
	friend void swap(UIComponent& c1, UIComponent& c2);

	UIComponent& operator=(UIComponent rhs);

	virtual std::vector<const UIComponent*> getChildren() const {return {};}

	// cb must have been started already
	void draw(const VkCommandBuffer& cb) const;
	void listenMousePos(UICoord mousepos, void* data);
	void listenMouseClick(bool click, void* data);

	static void setDefaultGraphicsPipeline(const UIPipelineInfo& p) {defaultgraphicspipeline = p;}
	static UIPipelineInfo getDefaultGraphicsPipeline() {return defaultgraphicspipeline;}
	static void setNoTex(UIImageInfo i) {notex = i;}
	static UIImageInfo getNoTex() {return notex;}
	static void setDefaultDS(VkDescriptorSet d) {defaultds = d;}
	static VkDescriptorSet getDefaultDS() {return defaultds;}
	static void setDefaultDrawFunc(dfType ddf) {defaultDrawFunc = ddf;}
	void setOnClickBegin(cfType f) {onClickBegin = f;}
	void setOnHoverEnd(cfType f) {onHoverEnd = f;}
	static void setScreenExtent(VkExtent2D e) {screenextent = e;}
	// TODO: phase out in favor of pass-by-reference
	UIPushConstantData* getPCDataPtr() {return &pcdata;}
	const UIPushConstantData& getPCData() const {return pcdata;}
	// also changes position of children
	void setPos(UICoord p);
	UICoord getPos() const {return pcdata.position;}
	void setExt(UICoord e) {pcdata.extent = e;}
	UICoord getExt() const {return pcdata.extent;}
	// also sets childrens' graphics pipelines
	void setGraphicsPipeline(const UIPipelineInfo& p);
	const UIPipelineInfo& getGraphicsPipeline() const {return graphicspipeline;}
	virtual void setDS(VkDescriptorSet d) {ds = d;}
	const VkDescriptorSet& getDS() const {return ds;}
	// TODO: phase out in favor of pass-by-reference
	VkDescriptorSet* getDSPtr() {return &ds;}
	void setDisplayFlag(UIDisplayFlags f) {display |= f;}
	void unsetDisplayFlag(UIDisplayFlags f) {display &= ~f;}
	void show();
	void hide();

protected:
	UIPushConstantData pcdata;
	static VkExtent2D screenextent;
	UIDisplayFlags display;
	UIPipelineInfo graphicspipeline;
	VkDescriptorSet ds;

	virtual std::vector<UIComponent*> _getChildren() {return {};}

private:
	dfType drawFunc;
	cfType onHover, onHoverBegin, onHoverEnd,
		onClick, onClickBegin, onClickEnd;
	UIEventFlags events;
	static UIPipelineInfo defaultgraphicspipeline;
	static UIImageInfo notex;
	static VkDescriptorSet defaultds;
	static dfType defaultDrawFunc;
	static cfType defaultOnHover, defaultOnHoverBegin, defaultOnHoverEnd, 
			defaultOnClick, defaultOnClickBegin, defaultOnClickEnd;
};

class UIContainer : public UIComponent {
public:
	UIContainer() = default;
	UIContainer(const UIContainer& rhs) :
		children(rhs.children),
		UIComponent(rhs) {};
	~UIContainer();

	std::vector<const UIComponent*> getChildren() const;
	/*
	 * This template function is a little hack to get the appropriate contructor called for classes like
	 * UIText, which needs to monitor how many objects are using which texture. Implicitly, T should
	 * only be a UIComponent inheritor, I'm unsure if there is a way to enforce this.
	 *
	 * It returns the heap-alloc'd pointer for further ops. Allows for a UIHandler to keep all these pointers straight itself if it needs to, without overhead in here.
	 */
	template<class T>
	T* addChild(const T& c) {
		T* temp = new T;
		std::cout << temp << std::endl;
		*temp = c;
		children.push_back(dynamic_cast<UIComponent*>(temp));
		return temp;
	}

private:
	std::vector<UIComponent*> _getChildren();
	
	// heap-alloc'd pointer vector, alloc'd and freed by UIContainer, so that we can have any type of
	// UIComponent
	std::vector<UIComponent*> children;
};

class UIImage : public UIComponent {
public:
	// instead of making these public, could add public intermediary functions to UIImage
	static tfType texLoadFunc;
	static tdfType texDestroyFunc;

	// Note: default constructor does not initialize the texture
	UIImage();
	UIImage(const UIImage& rhs);
	UIImage(UIImage&& rhs) noexcept;
	UIImage(UICoord p); 
	~UIImage();

	friend void swap(UIImage& t1, UIImage& t2);

	UIImage& operator=(UIImage rhs);

	std::vector<const UIComponent*> getChildren() const {return {};}
	virtual void setDS(VkDescriptorSet d) {ds = d;}
	const UIImageInfo& getTex() {return tex;}
	void setTex(const UIImageInfo& i);

	static void setTexLoadFunc(tfType tf) {texLoadFunc = tf;}
	static void setTexDestroyFunc(tdfType tdf) {texDestroyFunc = tdf;}

private:
	UIImageInfo tex;

	std::vector<UIComponent*> _getChildren() {return {};}

	static std::map<VkImage, uint8_t> imgusers;
};

class UIText : public UIImage {
public:
	// Note: default constructor does not initialize the texture
	UIText();
	UIText(const UIText& rhs) :
		text(rhs.text),
		UIImage(rhs) {}
	UIText(UIText&& rhs) noexcept :
		text(std::move(rhs.text)),
		UIImage(rhs) {}
	UIText(std::wstring t);
	UIText(std::wstring t, UICoord p); 
	~UIText() = default;

	friend void swap(UIText& t1, UIText& t2);

	UIText& operator=(UIText rhs);

	void setDS(VkDescriptorSet d);
	void setText(std::wstring t);
	const std::wstring& getText() {return text;}

private:
	std::wstring text;

	void genTex();

	static FT_Library ft;
	static FT_Face typeface;

	static constexpr FT_Pos truncate26_6(FT_Pos x) {return x >> 6;}
};

class UIDropdown : public UIComponent {
public:
	UIDropdown() : 
		options({}),
		unfolded(false),
		otherpos({0, 0}),
		otherext({0, 0}),
		UIComponent() {}
	UIDropdown(const UIDropdown& rhs) :
		options(rhs.options),
		unfolded(rhs.unfolded),
		otherpos(rhs.otherpos),
		otherext(rhs.otherext),
		UIComponent(rhs) {}
	UIDropdown(UIDropdown&& rhs) noexcept;
	UIDropdown(std::vector<std::wstring> o);
	UIDropdown(std::vector<std::wstring> o, UICoord p, UICoord e);

	std::vector<const UIComponent*> getChildren() const;
	void setPos(UICoord p);
	void setExt(UICoord e);
	void fold();
	void unfold();

protected:
	bool unfolded;
	UICoord otherpos, otherext;
	std::vector<UIText> options;

	std::vector<UIComponent*> _getChildren();

private:
	void setOptions(std::vector<std::wstring>& o);
};

class UIDropdownButtons : public UIDropdown {
public:
	UIDropdownButtons() = default;
	UIDropdownButtons(const UIDropdownButtons& rhs) :
		title(rhs.title),
		UIDropdown(rhs) {}
	UIDropdownButtons(UIDropdownButtons&& rhs) noexcept;
	UIDropdownButtons(std::wstring t);
	UIDropdownButtons(std::wstring t, std::vector<std::wstring> o);

	std::vector<const UIComponent*> getChildren() const;

private:
	UIText title;

	std::vector<UIComponent*> _getChildren();
};

class UIDropdownSelector : public UIDropdown {
public:
	UIDropdownSelector() : selected(nullptr) {}

private:
	UIText* selected;
};

class UIRibbon : public UIComponent {
public:
	UIRibbon();

	std::vector<const UIComponent*> getChildren() const;

	void addOption(std::wstring name);
	void addOption(UIDropdownButtons&& o);
	void addOption(std::wstring t, std::vector<std::wstring> o);
	// TODO: get rid of this function, should be using getChildren() instead
	std::vector<UIDropdownButtons> getOptions() {return options;}

private:
	std::vector<UIDropdownButtons> options;

	std::vector<UIComponent*> _getChildren();
};
