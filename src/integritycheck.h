/*
 * integritycheck.h
 *
 * (c) 2026 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file integritycheck.h
 * Header file for IntegrityCheck
 */

#ifndef INTEGRITYCHECK_H
#define INTEGRITYCHECK_H

#include <QList>
#include <QString>

/**
 * Read-only structural validation of a PortaBase file against the file
 * format's invariants (see docs/wiki/format.md and
 * docs/intent/integrity-check/integrity-check-design.md). Opens the file's
 * Metakit storage read-only, runs the version-gated invariant catalog, and
 * reports findings; it never writes to the file. Deliberately reads raw
 * Metakit views rather than going through Database, whose accessors assume
 * the very invariants being checked (and whose constructor upgrades
 * old-format files in place).
 */
class IntegrityCheck
{
public:
    /** How severe a finding is; see the design doc's severity principle. */
    enum Severity : int {
        Warning, /**< Tolerated by the application; degrades behavior */
        Error    /**< Relied on unguarded by some released version; crash or
                      data-loss risk */
    };

    /** Overall result of checking one file. */
    enum FileStatus : int {
        Clean,       /**< Readable, no findings */
        HasFindings, /**< Readable, at least one finding */
        Unreadable   /**< Not parseable as a PortaBase file */
    };

    /** One invariant violation found in the file. */
    struct Finding {
        QString checkId;   /**< EARS spec ID of the violated invariant */
        Severity severity; /**< Whether the violation is an error or warning */
        QString location;  /**< Which view/row/column/name it was found at */
        QString message;   /**< Human-readable description */
    };

    /** One check skipped because a prerequisite check failed. */
    struct SkippedCheck {
        QString checkId;        /**< The check that was skipped */
        QString prerequisiteId; /**< The failed check that blocked it */
    };

    explicit IntegrityCheck(const QString &filePath);

    QString filePath() const;
    FileStatus status() const;
    int formatVersion() const;
    bool encryptedContentsSkipped() const;
    QList<Finding> findings() const;
    QList<SkippedCheck> skippedChecks() const;
    QString report() const;

    static int exitCode(const QList<FileStatus> &statuses);

private:
    QString path; /**< Path of the file being checked */
    FileStatus fileStatus; /**< Overall result of the check */
    int version; /**< Declared format version, -1 if the file was unreadable */
    bool encryptedSkipped; /**< True if encrypted contents were not checked */
    QList<Finding> foundProblems; /**< The invariant violations found */
    QList<SkippedCheck> skipped; /**< Checks skipped due to failed prerequisites */
};

#endif
