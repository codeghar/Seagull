<?xml version="1.0"?>
<!--
  Copyright 2002-2004 The Apache Software Foundation or its licensors,
  as applicable.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import href="../../common/css/forrest.css.xslt"/>

  <!-- This is not used by Forrest but makes it possible to debug the
       stylesheet in standalone editors -->
  <xsl:output method = "text"  omit-xml-declaration="yes"  />

<xsl:template match="skinconfig">
  <xsl:apply-imports/>
<!-- rounded corners -->
#toptabs td{
	background-image: url('images/rc-t-l-10-1header-2tab-unselected-3tab-unselected.png');
<!-- this enables double rounded corners -->
<!--
    margin: 0px;
    padding:0px;
	border-bottom: 1px solid #ffffff;
	border-right: 1px solid #ffffff;
	border-left: 1px solid #ffffff;

}
#toptabs td a{
	background-image: url('images/rc-t-r-10-1header-2tab-unselected-3tab-unselected.png');

    background-repeat: no-repeat;
    background-position: top right;
    margin: 0px;
    padding-right: 10px;
    padding-left: 10px;
    padding-top: 0px;
    padding-bottom: 0px;
-->
}

#toptabs th {
	background-image: url('images/rc-t-l-10-1header-2tab-selected-3tab-selected.png');
}

</xsl:template>

<xsl:template match="group-logo">
  <xsl:if test="@width and @height">
#cn {
	background-image: url('../<xsl:value-of select="."/>');
	display: block;
	height: <xsl:value-of select="@height"/>px;
	width: <xsl:value-of select="@width"/>px;
}
  </xsl:if>
</xsl:template>

<xsl:template match="host-logo">
  <xsl:if test="@width and @height">
#poweredby {
	background-image: url('../<xsl:value-of select="."/>');
	display: block;
	height: <xsl:value-of select="@height"/>px;
	width: <xsl:value-of select="@width"/>px;
}
  </xsl:if>
</xsl:template>

<xsl:template match="project-logo">
  <xsl:if test="@width and @height">
#sc {
	background-image: url('../<xsl:value-of select="."/>');
	display: block;
	height: <xsl:value-of select="@height"/>px;
	width: <xsl:value-of select="@width"/>px;
}
  </xsl:if>
</xsl:template>


<xsl:template match="color[@name='header']">
#banner, #banner td, #toptabs { background-color: <xsl:value-of select="@value"/>;}
#toptabs td { border-left: 1px solid <xsl:value-of select="@value"/>;}
#toptabs th { border-left: 1px solid <xsl:value-of select="@value"/>;}
#banner { border-top: 1px solid <xsl:value-of select="@highlight"/>;}
</xsl:template>

<xsl:template match="color[@name='tab-selected']">
.tabs th  {
    background-color: <xsl:value-of select="@value"/>;
    border-right: 1px solid <xsl:value-of select="@lowlight"/>;
	color: <xsl:value-of select="@font"/>;}
.tabs th a:link{
	color: <xsl:value-of select="@link"/>;}
.tabs th a:visited {
	color: <xsl:value-of select="@vlink"/>;}
.tabs th a:hover {
	color: <xsl:value-of select="@hlink"/> !important; }
#mytools .label, #projecttools .label, #admintools .label, #communitytools .label {
	background-color: <xsl:value-of select="@value"/>;}
#mytools, #projecttools, #admintools, #communitytools {
	background-color: <xsl:value-of select="@value"/>;
	border-right: 1px solid <xsl:value-of select="@lowlight"/>;
	border-bottom: 1px solid <xsl:value-of select="@lowlight"/>;
}
.tabs {	border-bottom: 6px <xsl:value-of select="@value"/> solid; }</xsl:template>
<xsl:template match="color[@name='tab-unselected']">
.tabs td {
   background-color: <xsl:value-of select="@value"/>;
   color: <xsl:value-of select="@font"/>;}
.tabs td a:link{
	color: <xsl:value-of select="@link"/>;}
.tabs td a:visited {
	color: <xsl:value-of select="@vlink"/>;}
.tabs td a:hover {
	color: <xsl:value-of select="@hlink"/> !important;}
    </xsl:template>
<xsl:template match="color[@name='subtab-selected']">
#breadcrumbs a:link{
	color: <xsl:value-of select="@link"/>;}
#breadcrumbs a:visited {
	color: <xsl:value-of select="@vlink"/>;}
</xsl:template>
<xsl:template match="color[@name='subtab-unselected']">
#breadcrumbs { color: <xsl:value-of select="@font"/>;}
#breadcrumbs {background-color: <xsl:value-of select="@value"/>;
              border-top: 1px solid <xsl:value-of select="@highlight"/>;}
#main        {border-top: 1px solid <xsl:value-of select="@lowlight"/>;}
#breadcrumbs a:link{
	color: <xsl:value-of select="@link"/>;}
#breadcrumbs a:visited {
	color: <xsl:value-of select="@vlink"/>;}
#breadcrumbs a:hover {
	color: <xsl:value-of select="@hlink"/> !important;}
</xsl:template>

<xsl:template match="color[@name='heading']">

<xsl:choose>
    <xsl:when test="../../headings/@type='underlined'">
.app h3 {
    background-color: transparent;
    color: <xsl:value-of select="../color[@name='body']/@font"/>;
	border-bottom: 2px solid <xsl:value-of select="@value"/>;
}
    </xsl:when>
    <xsl:when test="../../headings/@type='boxed'">
.app h3 {
    background-color: <xsl:value-of select="@value"/>;
	background-image: url('images/rc-t-l-10-1body-2heading-3heading.png');
}
    </xsl:when>
    <xsl:otherwise>
.app h3 {
    color: <xsl:value-of select="../color[@name='body']/@font"/>;
	background-color: transparent;
}
    </xsl:otherwise>
</xsl:choose>


</xsl:template>
<xsl:template match="color[@name='subheading']">

<xsl:choose>
    <xsl:when test="../../headings/@type='underlined'">
.app h4 {
    background-color: transparent;
    color: <xsl:value-of select="../color[@name='body']/@font"/>;
	border-bottom: 1px solid <xsl:value-of select="@value"/>;
}
    </xsl:when>
    <xsl:when test="../../headings/@type='boxed'">
.app h4 {
    background-color: <xsl:value-of select="@value"/>;
	background-image: url('images/rc-t-l-10-1body-2subheading-3subheading.png');
}
    </xsl:when>
    <xsl:otherwise>
.app h4 {
    color: <xsl:value-of select="../color[@name='body']/@font"/>;
	background-color: transparent;
}
    </xsl:otherwise>
</xsl:choose>

</xsl:template>

<xsl:template match="color[@name='navstrip']">
#topmodule    { background-color: <xsl:value-of select="@value"/>;
                border-top: 1px solid <xsl:value-of select="@highlight"/>;
	            border-bottom: 1px solid <xsl:value-of select="@lowlight"/>;
                color: <xsl:value-of select="@font"/>;}
#topmodule #issueid {
	border-right: 1px solid <xsl:value-of select="@lowlight"/>;
}
#topmodule a:link{
	color: <xsl:value-of select="@link"/>;}
#topmodule a:visited {
	color: <xsl:value-of select="@vlink"/>;}
#topmodule a:hover {
	color: <xsl:value-of select="@hlink"/> !important; }
</xsl:template>
<xsl:template match="color[@name='toolbox']">
.toolgroup .label {	background: <xsl:value-of select="@value"/>;
                    border-bottom: 1px solid <xsl:value-of select="@lowlight"/>;
	                border-right: 1px solid <xsl:value-of select="@lowlight"/>;
	                color: <xsl:value-of select="@font"/>;
} </xsl:template>
<xsl:template match="color[@name='menu']">
#mytools .body, #projecttools .body, #admintools .body, #communitytools .body {
	background-color: <xsl:value-of select="@value"/>;
	border-right: 1px solid  <xsl:value-of select="@highlight"/>;
	border-bottom: 1px solid  <xsl:value-of select="@highlight"/>;
	border-top: 1px solid  <xsl:value-of select="@lowlight"/>;
	border-left: 1px solid  <xsl:value-of select="@lowlight"/>;
    }
#mytools .body a:link, #projecttools .body a:link, #admintools .body a:link, #communitytools .body a:link{
	color: <xsl:value-of select="@link"/>;}
#mytools .body a:visited, #projecttools .body a:visited, #admintools .body a:visited, #communitytools .body a:visited{
	color: <xsl:value-of select="@vlink"/>;}
#mytools .body a:hover, #projecttools .body a:hover, #admintools .body a:hover, #communitytools .body a:hover{
	color: <xsl:value-of select="@vlink"/> !important;}
</xsl:template>
<xsl:template match="color[@name='dialog']">
.toolgroup { background: <xsl:value-of select="@value"/>;}
.toolgroup .body {
	border-right: 1px solid <xsl:value-of select="@lowlight"/>;;
	border-bottom: 1px solid <xsl:value-of select="@lowlight"/>;;
}
</xsl:template>
<xsl:template match="color[@name='body']">
body         { background-color: <xsl:value-of select="@value"/>;}
a:link, #navcolumn a:visited, .app a:visited, .tasknav a:visited {
	color: <xsl:value-of select="@link"/>;
}
a:link.selfref, a:visited.selfref {
	color: <xsl:value-of select="@vlink"/> !important;
	text-decoration: none;
}
a:active, a:hover, #leftcol a:active, #leftcol a:hover {
	color: <xsl:value-of select="@hlink"/> !important;
}
</xsl:template>

<xsl:template match="color[@name='footer']">
#footer      { background-color: <xsl:value-of select="@value"/>;} </xsl:template>


<!-- ==================== other colors ============================ -->
<xsl:template match="color[@name='highlight']">
.highlight        { background-color: <xsl:value-of select="@value"/>;} </xsl:template>
<xsl:template match="color[@name='fixme']">
.warningmessage {border-color: <xsl:value-of select="@value"/>;} </xsl:template>
<xsl:template match="color[@name='note']">
.infomessage {  border-color: <xsl:value-of select="@value"/>;} </xsl:template>
<xsl:template match="color[@name='warning']">
.errormessage{  border-color: <xsl:value-of select="@value"/>;} </xsl:template>
<xsl:template match="color[@name='code']">
.code, code, pre        { border-color: <xsl:value-of select="@value"/>;} </xsl:template>
<xsl:template match="color[@name='table']">
body .grid td {
	border-top: 1px solid <xsl:value-of select="@value"/>;
	border-left: 1px solid <xsl:value-of select="@value"/>;}
.app th {
	background-color: <xsl:value-of select="@value"/>;}</xsl:template>
<xsl:template match="color[@name='table-cell']">
body .grid td {
	background-color: <xsl:value-of select="@value"/>;
} </xsl:template>

</xsl:stylesheet>