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
<!--
This stylesheet contains the majority of templates for converting documentv11
to HTML.  It renders XML as HTML in this form:

  <div class="content">
   ...
  </div>

..which site2xhtml.xsl then combines with HTML from the index (book2menu.xsl)
and tabs (tab2menu.xsl) to generate the final HTML.

-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:import href="../../../common/xslt/html/document-to-html.xsl"/>

  <xsl:template match="document">
    <meta-data>
      <xsl:apply-templates select="header/meta"/>
    </meta-data>
    <div class="content">
     <div id="topmodule" align="right">
      <table border="0" cellspacing="0" cellpadding="3" width="100%">
       <!-- ( ================= middle NavBar ================== ) -->
        <tr>
        <td class="tasknav" >
          <div align="left">
           <!--
                site2html.xsl will search for this as

                   td[@class='tasknav']/div[@align='left']

                to insert the breadcrumbs.
          -->
          </div>
        </td>
        <td id="issueid" class="tasknav">
        <div align="right">
          <div id="skinconf-printlink"/>
          <xsl:if test="$dynamic-page='false'">
            <div id="skinconf-podlink"/>
            <div id="skinconf-txtlink"/>
            <div id="skinconf-pdflink"/>
            <div id="skinconf-xmllink"/>
          </xsl:if>
        </div>
        </td>
       </tr>
      </table>
     </div>

    <!-- ( ================= Content================== ) -->
    <div id="bodycol">
      <xsl:if test="normalize-space(header/title)!=''">
        <div id="apphead">
         <h2><em><xsl:value-of select="header/title"/></em></h2>
        </div>
      </xsl:if>
      <xsl:if test="normalize-space(header/subtitle)!=''">
        <h3><em><xsl:value-of select="header/subtitle"/></em></h3>
      </xsl:if>
      <xsl:if test="header/abstract">
        <div class="abstract">
          <xsl:value-of select="header/abstract"/>
        </div>
      </xsl:if>

      <div id="projecthome" class="app">
        <xsl:apply-templates select="body"/>

       <xsl:if test="header/authors">
        <p align="right">
          <font size="-2">
            <xsl:for-each select="header/authors/person">
              <xsl:choose>
                <xsl:when test="position()=1">by&#160;</xsl:when>
                <xsl:otherwise>,&#160;</xsl:otherwise>
              </xsl:choose>
              <xsl:value-of select="@name"/>
            </xsl:for-each>
          </font>
        </p>
      </xsl:if>
       </div>
      </div>
    </div>
  </xsl:template>

  <xsl:template match="body">
    <div id="skinconf-toc-page"/>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template name="generate-id">
    <xsl:choose>
      <xsl:when test="@id">
        <xsl:value-of select="@id"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="generate-id(.)"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

    <xsl:template match="link | jump | fork | source | anchor | icon | code | figure | @id" >
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="section">
    <a name="{generate-id()}"/>

    <xsl:apply-templates select="@id"/>

    <xsl:variable name = "level" select = "count(ancestor::section)+1" />

    <xsl:choose>
      <xsl:when test="$level=1">
       <div class="h3">
        <h3><xsl:value-of select="title"/></h3>
        <xsl:apply-templates/>
      </div>
      </xsl:when>
      <xsl:when test="$level=2">
       <div class="h4">
        <h4><xsl:value-of select="title"/></h4>
        <xsl:apply-templates select="*[not(self::title)]"/>
      </div>

      </xsl:when>
      <!-- If a faq, answer sections will be level 3 (1=Q/A, 2=part) -->
      <xsl:when test="$level=3 and $notoc='true'">
        <h4 class="faq"><xsl:value-of select="title"/></h4>
        <div align="right"><a href="#{@id}-menu">^</a></div>
        <div style="margin-left: 15px">
          <xsl:apply-templates select="*[not(self::title)]"/>
        </div>
      </xsl:when>
      <xsl:when test="$level=3">
        <h4><xsl:value-of select="title"/></h4>
        <xsl:apply-templates select="*[not(self::title)]"/>

      </xsl:when>

      <xsl:otherwise>
        <h5><xsl:value-of select="title"/></h5>
        <xsl:apply-templates select="*[not(self::title)]"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="fixme">
    <div class="warningmessage">
      <p><strong>Fixme (<xsl:value-of select="@author"/>)</strong></p>
      <xsl:apply-templates/>
    </div>
  </xsl:template>

  <xsl:template match="note">
    <div class="infomessage">
      <p><strong>Note</strong></p>
      <xsl:apply-templates/>
    </div>
  </xsl:template>

  <xsl:template match="warning">
    <div class="errormessage">
      <p><strong>Warning</strong></p>
      <xsl:apply-templates/>
    </div>
  </xsl:template>

  <xsl:template match="p[@xml:space='preserve']">
    <xsl:apply-imports/>
  </xsl:template>

  <xsl:template match="table">
   <div class="h4">
    <xsl:apply-templates select="@id"/>
    <xsl:if test="caption">
        <h4><xsl:value-of select="caption"/></h4>
    </xsl:if>
    <xsl:apply-templates select="caption"/>
      <table border="1" cellspacing="2" cellpadding="3" width="100%" class="grid">
        <xsl:copy-of select="@cellspacing | @cellpadding | @border | @class | @bgcolor"/>
        <xsl:apply-templates/>
    </table>
   </div>
  </xsl:template>

  <!-- do not show caption or title elements, they are already in other places -->
  <xsl:template match="caption | title"/>
</xsl:stylesheet>
