<?xml version="1.0"?>
<!--
  Copyright 2002-2005 The Apache Software Foundation or its licensors,
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
        
    <xsl:template match="skinconfig">

  <xsl:if test="not(colors)">
  <colors>
  <!-- Collabnet --> 
    <color name="header"    value="#ffffff"/><!-- 003366 -->

    <color name="tab-selected" value="#dddddd" link="#555555" vlink="#555555" hlink="#555555"/>
    <color name="tab-unselected" value="#999999" link="#ffffff" vlink="#ffffff" hlink="#ffffff"/>
    <color name="subtab-selected" value="#cccccc" link="#000000" vlink="#000000" hlink="#000000"/>
    <color name="subtab-unselected" value="#cccccc" link="#555555" vlink="#555555" hlink="#555555"/>

    <color name="heading" value="#003366"/>
    <color name="subheading" value="#888888"/>
    
    <color name="navstrip" value="#dddddd" font="#555555"/>
    <color name="toolbox" value="#dddddd" font="#555555"/>
    <color name="border" value="#999999"/>
    
    <color name="menu" value="#ffffff"/>    
    <color name="dialog" value="#eeeeee"/>
            
    <color name="body"      value="#ffffff"/>
    
    <color name="table" value="#ccc"/>    
    <color name="table-cell" value="#ffffff"/>   
    <color name="highlight" value="#ffff00"/>
    <color name="fixme" value="#cc6600"/>
    <color name="note" value="#006699"/>
    <color name="warning" value="#990000"/>
    <color name="code" value="#003366"/>
        
    <color name="footer" value="#ffffff"/>
  </colors>
  </xsl:if>

     <xsl:copy>
      <xsl:copy-of select="@*"/>
      <xsl:copy-of select="node()[not(name(.)='colors')]"/>     
      <xsl:apply-templates select="colors"/>
     </xsl:copy> 
           
    </xsl:template>

    <xsl:template match="colors">
 
    <xsl:copy>
     <xsl:copy-of select="@*"/>
     <xsl:copy-of select="node()[name(.)='color']"/>     
     
     <xsl:if test="not(color[@name='header'])">
       <color name="header" value="#ffffff"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='tab-selected'])">
      <color name="tab-selected" value="#dddddd" link="#555555" vlink="#555555" hlink="#555555"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='tab-unselected'])">
      <color name="tab-unselected" value="#999999" link="#ffffff" vlink="#ffffff" hlink="#ffffff"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='subtab-selected'])">
      <color name="subtab-selected" value="#cccccc" link="#000000" vlink="#000000" hlink="#000000"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='subtab-unselected'])">
      <color name="subtab-unselected" value="#cccccc" link="#555555" vlink="#555555" hlink="#555555"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='heading'])">
      <color name="heading" value="#003366"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='subheading'])">
      <color name="subheading" value="#888888"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='navstrip'])">
      <color name="navstrip" value="#dddddd" font="#555555"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='toolbox'])">
      <color name="toolbox" value="#dddddd" font="#555555"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='border'])">
       <color name="border" value="#999999"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='menu'])">
       <color name="menu" value="#ffffff"/>    
     </xsl:if>  
     <xsl:if test="not(color[@name='dialog'])">
      <color name="dialog" value="#eeeeee"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='body'])">
      <color name="body" value="#ffffff"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='table'])">
      <color name="table" value="#cccccc"/>    
     </xsl:if>  
     <xsl:if test="not(color[@name='table-cell'])">
      <color name="table-cell" value="#ffffff"/>    
     </xsl:if>  
     <xsl:if test="not(color[@name='highlight'])">
       <color name="highlight" value="#ffff00"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='fixme'])">
       <color name="fixme" value="#cc6600"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='note'])">
       <color name="note" value="#006699"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='warning'])">
       <color name="warning" value="#990000"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='code'])">
       <color name="code" value="#003366"/>
     </xsl:if>  
     <xsl:if test="not(color[@name='footer'])">
       <color name="footer" value="#ffffff"/>
     </xsl:if>  
     </xsl:copy> 

    </xsl:template>
    
</xsl:stylesheet>
