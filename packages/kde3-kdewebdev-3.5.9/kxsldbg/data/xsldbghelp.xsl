<?xml version="1.0" encoding="UTF-8" ?>
<!-- 
     File : xsldoc.xsl     
     Author: Keith Isdale <k_isdale@tpg.com.au>
     Description: Stylesheet to process xsldoc.xml and generate help text
     Copyright Reserved Under GPL     
-->
<!-- This file does not require translation -->
<!-- NO TRANSLATION -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">
  <xsl:output method="text"/>
  <xsl:strip-space elements="text"/>
  <!-- The selected nodes to be printed for overview -->
  <xsl:variable name="overview_node" select="//chapter[@id='introduction']"/>
  <!-- The selected nodes to be printed for usage overview -->
  <xsl:variable name="usage_node" select="//chapter[@id='using-xsldbg']"/>
  <!-- The list of valid xsldbg commands -->
  <xsl:variable name="command_nodes" select="//chapter[@id='commands']"/>
  <!-- The list of i18n paras -->
  <xsl:variable name="i18n_para" select="//i18n"/>
  <!-- What version is this document-->
  <xsl:variable name="doc_version" select="'3.3.0-1'"/>
  <!-- The default version of xsldbg -->
  <xsl:param name="xsldbg_version" select="'3.3.0'"/>
  <!-- We want 'help' to point to a invalid command if stylesheet
       user has not provided a value for 'help' param-->
  <xsl:param name="help" select="'_#_'"/>
  <xsl:variable name="help_id" select="concat($help,'_cmd')"/>
  <!-- Do we printout all documentation '1' if so '0' otherwise -->
  <xsl:param name="alldocs" select="0"/>
  <!-- The documentation we can find for 'help' user requires -->
  <xsl:variable name="help_cmd" select="$command_nodes/sect1[@id=$help_id or @shortcut=$help_id]"/>

  <!-- Our translatables -->
  <xsl:param name="xsldbgVerTxt" select="'xsldbg version'"/>
  <xsl:param name="helpDocVerTxt" select="'Help document version'"/>
  <xsl:param name="helpErrorTxt" select="'Help not found for command'"/>



  <!-- Main template-->
  <xsl:template match="/">
<xsl:text>         </xsl:text><xsl:value-of select="$xsldbgVerTxt"/><xsl:text> </xsl:text><xsl:value-of select="$xsldbg_version"/>
<xsl:text> 
</xsl:text>   
<xsl:text>         ====================</xsl:text><xsl:text> 
</xsl:text>  
    <xsl:choose>
    <xsl:when test="count($help_cmd) > 0" >
      <xsl:apply-templates select="$help_cmd" />
      <xsl:value-of select="$helpDocVerTxt"/><xsl:text> </xsl:text><xsl:value-of select="$doc_version"/><xsl:text> 
</xsl:text> 
      </xsl:when>
    <xsl:otherwise>
        <xsl:if test="$help !='_#_'">
	  <xsl:value-of select="$helpErrorTxt"/><xsl:text> </xsl:text>  
          <xsl:value-of select="$help"/>
	</xsl:if>
        <xsl:if test="$help ='_#_'">
	    <xsl:apply-templates select="$overview_node"/>
	    <xsl:text>
</xsl:text>
	    <xsl:apply-templates select="$usage_node"/>
	    <xsl:value-of select="$helpDocVerTxt"/><xsl:text> </xsl:text><xsl:value-of select="$doc_version"/><xsl:text> 
</xsl:text> 
	</xsl:if>
<xsl:text> 
</xsl:text>   
   </xsl:otherwise>
  </xsl:choose>
<xsl:text> 
</xsl:text> 
  </xsl:template>


   <!-- Convert title into something useful -->
  <xsl:template match="title">
<xsl:for-each select="ancestor::node()"><xsl:text>  </xsl:text></xsl:for-each><xsl:value-of select="."/>
<xsl:text> 
</xsl:text><xsl:for-each select="ancestor::node()"><xsl:text>  </xsl:text></xsl:for-each>
<xsl:value-of
  select="substring('____________________________________________________________',
  1, string-length())" />
<xsl:text> 
</xsl:text>
  </xsl:template>

  <xsl:template match="text()">
    <xsl:value-of select="normalize-space()"/>
  </xsl:template>

  <xsl:template match="row">
    <xsl:value-of select="$indentgroup/indent[@level=$indentcount]" />
    <xsl:apply-templates/>
  </xsl:template>
  
  <xsl:template match="para">
    <xsl:text> 
</xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="informaltable|table">
    <xsl:text>
</xsl:text>
    <xsl:apply-templates select="title"/>
    <xsl:for-each select="tgroup/tbody/row/entry|tbody/row/entry">
      <xsl:for-each select="ancestor::node()"><xsl:text>  </xsl:text></xsl:for-each><xsl:apply-templates/>
      <xsl:text>
</xsl:text>
    </xsl:for-each>
  </xsl:template>

</xsl:stylesheet>


<!-- initialization code for xemacs -->
<!--
Local Variables:
mode: xsl
sgml-minimize-attributes:nil
sgml-general-insert-case:lower
sgml-indent-step:2
sgml-indent-data:nil
End:
-->
