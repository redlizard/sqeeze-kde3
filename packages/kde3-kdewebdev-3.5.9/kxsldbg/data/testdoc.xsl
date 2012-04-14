<?xml version="1.0" encoding="UTF-8"?>
<!-- 
     File : testdoc.xsl     
     Author: Keith Isdale <k_isdale@tpg.com.au>
     Description: stylesheet for testing
     Copyright Reserved Under GPL     
-->
<!-- This file does not require translation -->
<!-- NO TRANSLATION -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

    <xsl:import href="test_import.xsl"/>
    <xsl:include href="test_include_top.xsl"/>
    <xsl:strip-space elements="text()"/>
    <xsl:decimal-format name="test" decimal-separator="."/>
    <xsl:output method="text"/>
    <xsl:variable name="globalvariable" select="'foo'"/>

    <xsl:template match="/">
	<xsl:call-template name="test_set_variable">
	    <xsl:with-param name="item" select="'1234'"/>
	</xsl:call-template>

	<xsl:variable name="localvariable" select="'bar'"/>
	<xsl:text>Global variable contains </xsl:text><xsl:value-of select="$globalvariable"/><xsl:text>
</xsl:text>
	<xsl:text>Local variable contains </xsl:text><xsl:value-of select="$localvariable"/><xsl:text>
</xsl:text>    

	<!-- test import of xsl file -->
        <xsl:call-template name="import_top"/>

        <!-- Basic xsl:apply-templates, xsl:call-template usage -->
        <!-- Test basic usage of xsl:apply-templates -->
        <xsl:apply-templates select="//result/data"/>
        <!-- Test basic usage of xsl:call-template -->
        <xsl:call-template name="call-template1"/>

        <!-- Test xsl:apply-templates with parameter value. 
           Test the ability to step into a xsl:with-param child -->
        <xsl:apply-templates select="//result/data">
	      <xsl:with-param name="item">
		    <item/>
	      </xsl:with-param>
        </xsl:apply-templates>

        <!-- Test xsl:call-template with parameter value
             Test the ability to step into a xsl:with-param child -->
	<xsl:call-template name="call-template2">
	    <xsl:with-param name="item">
		 <item />
	     </xsl:with-param>
	</xsl:call-template>

        <!-- Test ability to step into xsl:param from xsl:apply-templates -->
        <xsl:apply-templates select="//result/extra" />

        <!-- Test ability to step into xsl:param from xsl:call-template -->
        <xsl:call-template name="call-template3" />        

        <!-- Test ability to step into xsl:sort from xsl:apply-templates -->
        <xsl:apply-templates select="//result/data">
	    <xsl:sort select="."/>
	    <xsl:text>
</xsl:text>
        </xsl:apply-templates>

        <xsl:apply-imports/> <!-- useless but test that we can step to it -->

       <xsl:apply-templates select="//result/data" mode="verbose" />

    </xsl:template>
  

    <xsl:template match="result">
	<xsl:param name="item" select="'default'"/>
	<!-- ignore node content -->
    </xsl:template>


    <xsl:template match="data">
	<!-- ignore node content -->
    </xsl:template>
    
    <xsl:template match="data" mode="verbose">
	<xsl:apply-templates />
    </xsl:template>


    <xsl:template match="extra">
	<xsl:param name="item">
	    <item/>
	</xsl:param>
	<!-- ignore node content -->
	<xsl:text>
</xsl:text>
    </xsl:template>


    <xsl:template name="call-template1">
	<xsl:number value="position()" format="1."/>
	<xsl:text>
</xsl:text>
    </xsl:template>


    <xsl:template name="call-template2">
	<!-- ignore any param provided -->
	<!-- test message -->
	<xsl:message terminate="no">Message here</xsl:message>
	<xsl:processing-instruction name="pitest">
	pi text
	</xsl:processing-instruction>
	<xsl:text>
</xsl:text>
    </xsl:template>


    <xsl:template name="call-template3">
	<xsl:param name="item">
	    <item/>
	</xsl:param>
	<!-- test comments -->
	<xsl:comment>A text comment.</xsl:comment>
	<!-- test copy and copy-of -->
	<xsl:copy>copy text</xsl:copy>
	<xsl:copy-of select="'copy-of Text'"/>
	<xsl:text>
</xsl:text>
    </xsl:template>


    <xsl:template name="test_set_variable">
	<xsl:param name="item" select="'default-value'"/>
	<xsl:value-of select="$item"/>
    <xsl:text>
</xsl:text>
    </xsl:template>

</xsl:stylesheet>
