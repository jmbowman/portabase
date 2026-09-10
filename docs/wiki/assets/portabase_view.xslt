<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
     xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/">
<html>
<head>
<title>PortaBase data</title>
</head>
<body>
  <table>
    <tr>
      <td><b>View:</b></td>
      <xsl:choose>
        <xsl:when test="/portabase/global/gview='_all'">
          <td>All Columns</td>
        </xsl:when>
        <xsl:otherwise>
          <td><xsl:value-of select="/portabase/global/gview"/></td>
        </xsl:otherwise>
      </xsl:choose>
    </tr>
    <tr>
      <td><b>Sorting:</b></td>
      <xsl:choose>
        <xsl:when test="/portabase/global/gsort='_single'">
          <td><xsl:apply-templates select="/portabase/sortcolumns/sortcolumn[scsort='_single']"/></td>
        </xsl:when>
        <xsl:otherwise>
          <td><xsl:value-of select="/portabase/global/gsort"/></td>
        </xsl:otherwise>
      </xsl:choose>
    </tr>
    <tr>
      <td><b>Filter:</b></td>
      <xsl:choose>
        <xsl:when test="/portabase/global/gfilter='_allrows'">
          <td>All Rows</td>
        </xsl:when>
        <xsl:otherwise>
          <td><xsl:value-of select="/portabase/global/gfilter"/></td>
        </xsl:otherwise>
      </xsl:choose>
    </tr>
  </table>
  <table border="1">
    <xsl:apply-templates select="/portabase/viewcolumns"/>
    <xsl:apply-templates select="/portabase/data"/>
  </table>
</body>
</html>
</xsl:template>

<xsl:template match="/portabase/sortcolumns/sortcolumn">
  <xsl:value-of select="scname"/><xsl:text>, </xsl:text>
  <xsl:if test="scdesc='0'">
    <xsl:text>Ascending Order</xsl:text>
  </xsl:if>
  <xsl:if test="scdesc='1'">
    <xsl:text>Descending Order</xsl:text>
  </xsl:if>
</xsl:template>

<xsl:template match="/portabase/viewcolumns">
  <tr style="color:blue">
    <xsl:apply-templates select="viewcolumn"/>
  </tr>
</xsl:template>

<xsl:template match="viewcolumn">
  <xsl:if test="vcview=/portabase/global/gview">
    <td><xsl:value-of select="vcname"/></td>
  </xsl:if>
</xsl:template>

<xsl:template match="/portabase/data">
  <xsl:apply-templates select="r[not(@h)]"/>
</xsl:template>

<xsl:template match="r">
  <tr>
    <xsl:apply-templates/>
  </tr>
</xsl:template>

<xsl:template match="s|i|f|n|e|c|q">
  <xsl:if test="not(@h)">
    <td><xsl:value-of select="."/></td>
  </xsl:if>
</xsl:template>

<xsl:template match="b">
  <xsl:if test="not(@h)">
    <xsl:if test="text()='1'">
      <td>Yes</td>
    </xsl:if>
    <xsl:if test="text()='0'">
      <td>No</td>
    </xsl:if>
  </xsl:if>
</xsl:template>

<xsl:template match="d|t">
  <xsl:if test="not(@h)">
    <td><xsl:value-of select="@s"/></td>
  </xsl:if>
</xsl:template>

<xsl:template match="p">
  <xsl:if test="not(@h)">
    <td><a><xsl:attribute name="href"><xsl:value-of select="."/></xsl:attribute><xsl:value-of select="."/></a></td>
  </xsl:if>
</xsl:template>

</xsl:stylesheet>
