// JavaScript Document
var WebTxts=new Array();
WebTxts[0]="未知";
WebTxts[1]="未找到";
WebTxts[2]="错误";

WebTxts[10]="确定";
WebTxts[11]="返回首页";
WebTxts[12]="返回设置页";

WebTxts[30]="配置成功";
WebTxts[31]="密码错误，请重新输入";
WebTxts[32]="配置成功，AP即将改变，请重新连接Wifi，重新登录！";
WebTxts[33]="IO配置成功，需重启后生效！";
WebTxts[34]="IO变量映射成功，需重启后生效！";
WebTxts[35]="杂项配置成功，需重启后生效！";
WebTxts[36]="重启成功，请重新登陆！";
WebTxts[37]="授权码错误，请向经销商获取";



//根据ID获取文本
function getWebTxt(IdStr)
{
	var Id=parseInt(IdStr);
	
	if(isNaN(Id)) return IdStr.toString();
	if(Id >= WebTxts.length) return WebTxts[1];
	return WebTxts[Id];
}

/*检查登陆授权*/
function authHandler(){
	var auth_num=getCookie("auth_num");
	if(auth_num != "${auth_num}") location.href="/";
}

/*处理tips*/
function txtHandler(){
	//显示tips
	var tips=getQueryString("tip");
	if(tips!=null && tips.toString().length>0)
	{
		$("#web_tip").show();
		$("#web_tip").text(getWebTxt(tips));
	}	
}

/*处理跳转地址*/
function jumpHandler(){
	var jump_url=getQueryString("url");
	if(jump_url!=null && jump_url.toString().length>0)
	{
		$("#jump_link").attr("href",jump_url);
	}
	
	var jump_btn=getQueryString("btn");
	if(jump_btn!=null && jump_btn.toString().length>0)
	{
		$("#jump_btn").val(getWebTxt(jump_btn));
	}
}

function rebootPost(){
	$.ajax({
		type:'POST',
        url:'/reboot.set',
        data:JSON.stringify({name: 'zepto.js'}),
        contentType:'application/json'
        });
	location.href="/?tip=36";	
}

/*处理菜单*/
function menuHandler(){	
	//填充菜单
	var menu_div = "<div id=\"page_menu\"></div>";
	menu_div = menu_div + "<div id=\"page_shade_site\"></div><div id=\"page_shade\"></div>";
	$("#page_hd").after(menu_div);
	
	var menu_str = "<ul>";
	menu_str = menu_str + "<li><a href=\"./wifi\">WIFI</a></li>";
	menu_str = menu_str + "<li><a href=\"./ap\">AP</a></li>";
	menu_str = menu_str + "<li><a href=\"./gpio\">GPIO</a></li>";
	menu_str = menu_str + "<li><a href=\"./vars\">VARS</a></li>";
	menu_str = menu_str + "<li><a href=\"./misc\">MISC</a></li>";
	menu_str = menu_str + "<li><a href=\"####\" onclick=\"rebootPost()\">Reboot</a></li>";
	menu_str = menu_str + "<li><a href=\"/\">Logout</a></li>";
	$("#page_menu").html(menu_str);

	//注册菜单点击事件	
	$("#page_sel").click(function(){		
		$("#page_menu li").css("background-color","#666");		
		$("#page_shade").toggle();
		$("#page_menu").toggle();
	});	
	
	$("#page_shade").click(function(){		
		$("#page_shade").toggle();
		$("#page_menu").toggle();
		$("#page_menu li").css("background-color","#666");
	});
	
	$("#page_menu li").click(function(){
		$(this).css("background-color","#999");
		$("#page_shade").toggle();
		$("#page_menu").toggle();
	});
}

/*获取随机数*/
function getRand() 
{     
    var TopLimit = 9999999;     
    var BottomLimit = 0;     
    return parseInt(Math.random() * (TopLimit - BottomLimit + 1) + BottomLimit);         
}

/* 设置cookie函数 */
//使用示例：
//setCookie("name","hayden","s20");
//s20是代表20秒
//h是指小时，如12小时则是：h12
//d是天数，30天则：d30
function setCookie(key,value,day) 
{
    var date=new Date();
    date.setDate(date.getDate()+day);
    document.cookie=key+'='+escape(value)+';expires='+date;
}

/* 获取cookie函数 */
function getCookie(key) 
{
    var coo=unescape(document.cookie);//解码
    var arr1=coo.split('; ');//第一次分解后是数组
    for (var i=0;i<arr1.length;i++){//第二次循环拆分数组
        var arr2=arr1[i].split('=');
        if(arr2[0]==key){
            return arr2[1];
        }
    }
}

/* 删除cookie */
function removeCookie(key) 
{
    setCookie(key,'',-1);
}

/*获取url参数*/
function getQueryString(name)
{
	var reg = new RegExp("(^|&)"+ name +"=([^&]*)(&|$)");
	var r = window.location.search.substr(1).match(reg);
	if(r!=null) return unescape(r[2]); 
	return null;
}

/*从字符串str中获取到name参数等号后面的值*/
function getStrVal(str,name)
{
	var reg = new RegExp("(^|&)"+ name +"=([^&]*)(&|$)");
	var r = str.match(reg);
	if(r!=null) return unescape(r[2]); 
	return null;
}

/*按钮点击效果*/
function btnClickColor(obj,color,jumpUrl){
	var orgColor=obj.style.backgroundColor;
	setTimeout(function(){
		obj.style.backgroundColor=orgColor;
	},300);
	
	obj.style.backgroundColor=color;
	if(jumpUrl!="")
	{
		if(jumpUrl=='-1'){window.history.back(-1);}
		else{location.href=jumpUrl;} 	
	}
}


<!--END-->